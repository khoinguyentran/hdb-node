#include "fsm.hpp"
#include "common.hpp"
#include "global.hpp"

#include <curl/curl.h>
#include <qp_port.h>
#include <glog/logging.h>
#include <pstream.h>

#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace app
{

using namespace boost::filesystem;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::shared_ptr;
using boost::make_shared;
using std::ofstream;
using std::ostringstream;
using std::string;
using std::vector;

struct vca_info
{    
    string name;
    string input;
    string params;
    boost::thread worker_thread;
};

class vca_manager : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
    QP::QTimeEvt vca_event_timer_;
    QP::QActive* controller_;
    QP::QActive* uploader_;

public:
    vca_manager();
    void log_vca_event ( gevt const * const );
    void handle_vca ( shared_ptr< vca_info > );
    void start_vca ( shared_ptr< vca_info > );
    void populate_vca_list();
    void get_snapshot ( string const &, path const & );
    static size_t get_snapshot_callback ( void *, size_t, size_t, void * );

private:
    vector< shared_ptr< vca_info > > vca_list_;

protected:
    static QP::QState initial ( vca_manager * const, QP::QEvt const * const );
    static QP::QState active ( vca_manager * const, QP::QEvt const * const );

};

// Constructor.
vca_manager::vca_manager()
    : QActive ( Q_STATE_CAST ( &vca_manager::initial ) ),
      timeout_ ( EVT_TIMEOUT ), vca_event_timer_ ( EVT_GENERATE_VCA_EVENT )
{
}

// initial
QP::QState vca_manager::initial ( vca_manager * const me, QP::QEvt const * const e )
{
    me->controller_ = global::get_default_controller();
    me->uploader_ = global::get_default_uploader();
    return Q_TRAN ( &vca_manager::active );
}

// active /
QP::QState vca_manager::active ( vca_manager * const me, QP::QEvt const * const e )
{
    QP::QState status;

    switch ( e->sig )
    {
    case Q_ENTRY_SIG:
    {
        LOG ( INFO ) << "VCA Manager started.";
        
        me->populate_vca_list();
        
        BOOST_FOREACH ( shared_ptr < vca_info > info, me->vca_list_ )
        {
            me->start_vca ( info );
        }
        
        status = Q_HANDLED();
        break;
    }
    
    case EVT_VCA_STOPPED:
    {
        string vca_name = static_cast < gevt const * const >(e)->args.get< string >("name");
        shared_ptr < vca_info > info;
        
        BOOST_FOREACH ( info, me->vca_list_ )
        {
            if ( info->name.compare(vca_name) == 0)
                break;
        }
        
        LOG ( INFO ) << "Restarting VCA [" << vca_name << "]...";
        me->start_vca ( info );
        
        status = Q_HANDLED();
        break;
    }

    case EVT_GENERATE_VCA_EVENT:
    {
        LOG ( INFO ) << "Generating vca event...";

        auto evt = Q_NEW ( gevt, EVT_VCA_EVENT );
        evt->args.put ( "description", "violation event at site001" );
        ptime timestamp ( microsec_clock::universal_time() );
        evt->args.put ( "timestamp", common::get_utc_string ( timestamp ) );
        evt->args.put ( "snapshot_path", "snapshots/vca.jpg" );

        LOG ( INFO ) << evt->args.get< string > ( "timestamp" ) << ": "
                     << evt->args.get< string > ( "description" ) << ": "
                     << evt->args.get< string > ( "snapshot_path" );
        
        me->postFIFO ( evt );

        status = Q_HANDLED();
        break;
    }

    case EVT_VCA_EVENT:
    {  
        me->log_vca_event ( static_cast < gevt const * const >(e) );

        status = Q_HANDLED();
        break;
    }

    default:
        status = Q_SUPER ( &QHsm::top );
        break;
    }

    return status;
}

void vca_manager::log_vca_event ( gevt const * const e )
{
    auto sql = global::get_database_handle();
    int error;

    ostringstream stmt;
    stmt << "INSERT INTO events VALUES(NULL, "
         << "'" << e->args.get< string > ( "timestamp" ) << "'" << ","
         << "'" << e->args.get< string > ( "description" ) << "'" << ", "
         << "'" << e->args.get< string > ( "snapshot_path" ) << "'" << ", "
         << "0" << ")";

    error = sqlite3_exec ( sql, stmt.str().c_str(), 0, 0, 0 );
    if ( error )
    {
        LOG ( ERROR ) << "Could not register new event with database.";
    }
}

void vca_manager::handle_vca ( shared_ptr< vca_info > info )
{
    ostringstream cmdline;
    uint16_t httpport = global::config()->get(info->name + ".httpport", 4001);
    cmdline << "vca/vca -i " << info->input
            << " " << info->params
            << " --httpport " << httpport;
            
    LOG ( INFO ) << info->name << ": " << cmdline.str();
    redi::ipstream in ( cmdline.str() );
    string line;
    while ( std::getline ( in, line ) )
    {           
        auto evt = Q_NEW ( gevt, EVT_VCA_EVENT );
        evt->args.put ( "description", line );
        ptime timestamp ( microsec_clock::universal_time() );
        evt->args.put ( "timestamp", common::get_utc_string ( timestamp ) );

        path local_folder ( global::config()->get ( "vca.snapshot_dir", "snapshots" ) );
        path local_file ( info->name + " " + common::get_utc_string ( timestamp ) + ".jpg" );
        path absolute=operator/ ( local_folder, local_file );
        evt->args.put ( "snapshot_path", absolute.string() );

        ostringstream url;
        url << "http://localhost:" << httpport << "/frame.jpg";

        get_snapshot ( url.str(), absolute );

        this->postFIFO ( evt );
    }
    
    boost::this_thread::sleep_for ( boost::chrono::seconds ( 3 ) );
    
    auto evt = Q_NEW ( gevt, EVT_VCA_STOPPED );
    evt->args.put ( "name", info->name );
    this->postFIFO ( evt );
}

void vca_manager::start_vca ( shared_ptr< vca_info > info )
{
    LOG ( INFO ) << "Starting VCA [" << info->name << "]...";
    info->worker_thread = boost::thread (
        boost::bind ( &vca_manager::handle_vca, this, info ) );
}

void vca_manager::populate_vca_list()
{
    BOOST_FOREACH ( ptree::value_type& section, *global::config() )
    {
        if ( section.first.find ( "vca-" ) == 0 )
        {
            auto info = make_shared < vca_info >();
            info->name = section.first.data();
            info->input = global::config()->get (
                section.first.data() + string ( ".input" ), "localhost" );
            info->params = global::config()->get (
                section.first.data() + string ( ".params" ), "-b -nomotiondetection" );
            this->vca_list_.push_back ( info );            
        }
    }
}

void
vca_manager::get_snapshot ( string const& url, path const& local_path )
{
    ofstream* ofs = new ofstream ( local_path.string(), ofstream::binary | ofstream::trunc );
    auto curl = curl_easy_init();
    CURLcode curl_error;
    
    if ( ofs != NULL && curl != NULL )
    {
        curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() );
        curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, &vca_manager::get_snapshot_callback );
        curl_easy_setopt ( curl, CURLOPT_WRITEDATA, ofs );
        curl_error = curl_easy_perform ( curl );
        curl_easy_cleanup ( curl );
        ofs->close();
    }
}

size_t
vca_manager::get_snapshot_callback ( void* ptr, size_t size, size_t nmemb, void* userdata )
{
    auto realsize = size * nmemb;
    auto ofs = ( ofstream* ) userdata;
    ofs->write ( ( const char* ) ptr, realsize );
    return realsize;
}

QP::QActive* create_vca_manager()
{
    return new vca_manager();
}

}
