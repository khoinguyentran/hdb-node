#include "fsm.hpp"
#include "common.hpp"
#include "global.hpp"

#include <curl/curl.h>
#include <qp_port.h>
#include <glog/logging.h>
#include <pstream.h>

#include "corecomm/DeviceManagementService.h"
#include "corecomm/StreamControlService.h"
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include <protocol/TBinaryProtocol.h>

#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <sstream>
#include <vector>

namespace app
{

using namespace boost::filesystem;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace com::kaisquare::core::thrift;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::scoped_ptr;
using boost::shared_ptr;
using boost::make_shared;
using std::ofstream;
using std::ostringstream;
using std::string;
using std::vector;

struct device_info
{
    string name;
    string model;
    string host;
    string port;
    string username;
    string password;
    string location;
    string latitude;
    string longitude;
};

struct vca_info
{
    string name;
    string analysis;
    shared_ptr< device_info > device;
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

    void start_vca ( vca_info & );
    void handle_vca_loitering ( vca_info & );
    void handle_vca_colorfulness ( vca_info & );

    void populate_device_list();
    void populate_vca_list();

    void get_snapshot ( string const &, path const & );
    string get_mjpeg_url ( device_info const & );
    static size_t get_snapshot_callback ( void *, size_t, size_t, void * );

    bool is_device_registered ( device_info const & );
    void update_device ( device_info const & );
    void register_device ( device_info const & );
    string get_device_id ( device_info const & );
    void handle_register_devices();

private:
    vector< shared_ptr< vca_info > > vca_list_;
    vector< shared_ptr< device_info > > device_list_;
    boost::thread worker_thread_;

protected:
    static QP::QState initial ( vca_manager * const, QP::QEvt const * const );
    static QP::QState active ( vca_manager * const, QP::QEvt const * const );
    static QP::QState idle ( vca_manager * const, QP::QEvt const * const );
    static QP::QState register_devices ( vca_manager * const, QP::QEvt const * const );
    static QP::QState run_vca ( vca_manager * const, QP::QEvt const * const );
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
    return Q_TRAN ( &vca_manager::register_devices );
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

        me->populate_device_list();
        me->populate_vca_list();

        status = Q_HANDLED();
        break;
    }

    default:
        status = Q_SUPER ( &QHsm::top );
        break;
    }

    return status;
}

// active / idle
QP::QState vca_manager::idle ( vca_manager * const me, QP::QEvt const * const e )
{
    QP::QState status;

    switch ( e->sig )
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
            break;
        }

        default:
            status = Q_SUPER ( &vca_manager::active );
            break;
    }

    return status;
}

// active / register_devices
QP::QState vca_manager::register_devices ( vca_manager * const me, QP::QEvt const * const e )
{
    QP::QState status;

    switch ( e->sig )
    {
        case Q_ENTRY_SIG:
        {
            me->worker_thread_ = boost::thread (
                boost::bind ( &vca_manager::handle_register_devices, me ) );

            status = Q_HANDLED();
            break;
        }

        case EVT_DEVICES_REGISTERED:
            status = Q_TRAN ( &vca_manager::run_vca );
            break;

        default:
            status = Q_SUPER ( &vca_manager::active );
            break;
    }

    return status;
}

// active / run_vca
QP::QState vca_manager::run_vca ( vca_manager * const me, QP::QEvt const * const e )
{
    QP::QState status;

    switch ( e->sig )
    {
        case Q_ENTRY_SIG:
        {
            LOG ( INFO ) << "Running vcas...";

            BOOST_FOREACH ( auto& vca, me->vca_list_ )
            {
                me->start_vca ( *vca );
            }

            status = Q_HANDLED();
            break;
        }

        case EVT_VCA_EVENT:
        {
            me->log_vca_event ( static_cast < gevt const * const >(e) );

            status = Q_HANDLED();
            break;
        }

        case EVT_VCA_STOPPED:
        {
            string vca_name = static_cast < gevt const * const >(e)->args.get< string >("name");

            BOOST_FOREACH ( auto& info, me->vca_list_ )
            {
                if ( info->name.compare ( vca_name ) == 0)
                {
                    LOG ( INFO ) << "Restarting VCA [" << vca_name << "]...";
                    me->start_vca ( *info );
                    break;
                }
            }

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

        default:
            status = Q_SUPER ( &vca_manager::active );
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
         << "'" << "vca" << "'" << ","
         << "'" << e->args.get< string > ( "description" ) << "'" << ", "
         << "'" << e->args.get< string > ( "snapshot_path" ) << "'" << ", "
         << "0" << ")";

    error = sqlite3_exec ( sql, stmt.str().c_str(), 0, 0, 0 );
    if ( error )
    {
        LOG ( ERROR ) << "Could not register new event with database.";
    }
}

void vca_manager::handle_vca_loitering ( vca_info& info )
{
    string url = get_mjpeg_url ( *info.device );

    ostringstream cmdline;
    uint16_t httpport = global::config()->get(info.name + ".httpport", 4001);
    cmdline << "vca/vca -i " << url
    << " " << info.params
    << " --httpport " << httpport;

    LOG ( INFO ) << info.name << ": " << cmdline.str();

    //redi::ipstream in ( cmdline.str() );
    //string line;
    //while ( std::getline ( in, line ) )
    //{
        //auto evt = Q_NEW ( gevt, EVT_VCA_EVENT );
        //evt->args.put ( "description", line );
        //ptime timestamp ( microsec_clock::universal_time() );
        //evt->args.put ( "timestamp", common::get_utc_string ( timestamp ) );

        //path local_folder ( global::config()->get ( "vca.snapshot_dir", "snapshots" ) );
        //path local_file ( info->name + " " + common::get_utc_string ( timestamp ) + ".jpg" );
        //path absolute=operator/ ( local_folder, local_file );
        //evt->args.put ( "snapshot_path", absolute.string() );

        //ostringstream url;
        //url << "http://localhost:" << httpport << "/frame.jpg";

        //get_snapshot ( url.str(), absolute );

        //this->postFIFO ( evt );
    //}

    //boost::this_thread::sleep_for ( boost::chrono::seconds ( 3 ) );

    //auto evt = Q_NEW ( gevt, EVT_VCA_STOPPED );
    //evt->args.put ( "name", info->name );
    //this->postFIFO ( evt );

}

void
vca_manager::handle_vca_colorfulness ( vca_info& info )
{
    string url = get_mjpeg_url ( *info.device );

    ostringstream cmdline;
    uint16_t httpport = global::config()->get(info.name + ".httpport", 4001);
    cmdline << "vca/vca -i " << url
    << " " << info.params
    << " --httpport " << httpport;

    LOG ( INFO ) << info.name << ": " << cmdline.str();

    redi::ipstream in ( cmdline.str() );
    string line;
    while ( std::getline ( in, line ) )
    {
        auto evt = Q_NEW ( gevt, EVT_VCA_EVENT );
        evt->args.put ( "description", line );
        ptime timestamp ( microsec_clock::universal_time() );
        evt->args.put ( "timestamp", common::get_utc_string ( timestamp ) );

        path local_folder ( global::config()->get ( "vca.snapshot_dir", "snapshots" ) );
        path local_file ( info.name + " " + common::get_utc_string ( timestamp ) + ".jpg" );
        path absolute=operator/ ( local_folder, local_file );
        evt->args.put ( "snapshot_path", absolute.string() );

        ostringstream url;
        url << "http://localhost:" << httpport << "/frame.jpg";

        get_snapshot ( url.str(), absolute );

        this->postFIFO ( evt );
    }

    boost::this_thread::sleep_for ( boost::chrono::seconds ( 3 ) );

    auto evt = Q_NEW ( gevt, EVT_VCA_STOPPED );
    evt->args.put ( "name", info.name );
    this->postFIFO ( evt );

}

void
vca_manager::start_vca ( vca_info& info )
{
    LOG ( INFO ) << "Starting VCA [" << info.name << "]...";

    auto func = &vca_manager::handle_vca_loitering;

    if ( info.analysis.compare ( "loitering" ) == 0 )
        func = &vca_manager::handle_vca_loitering;
    else if ( info.analysis.compare ( "colorfulness" ) == 0 )
        func = &vca_manager::handle_vca_colorfulness;

    info.worker_thread = boost::thread (
        boost::bind ( func, this, boost::ref ( info ) ) );
}

void
vca_manager::populate_device_list()
{
    BOOST_FOREACH ( ptree::value_type& section, *global::config() )
    {
        if ( section.first.find ("device") == 0 )
        {
            auto a_dev = make_shared< device_info >();
            a_dev->name = section.first.data();
            a_dev->host = global::config()->get (
                section.first.data() + string ( ".host" ), "localhost" );
            a_dev->model = global::config()->get (
                section.first.data() + string ( ".model" ), "Amtk IP Camera" );
            a_dev->port = global::config()->get (
                section.first.data() + string ( ".port" ), "80" );
            a_dev->username = global::config()->get (
                section.first.data() + string ( ".username" ), "admin" );
            a_dev->password = global::config()->get (
                section.first.data() + string ( ".password" ), "admin" );
            a_dev->location = global::config()->get (
                section.first.data() + string ( ".location" ), "N/A" );
            a_dev->latitude = global::config()->get (
                section.first.data() + string ( ".latitude" ), "0.0" );
            a_dev->longitude = global::config()->get (
                section.first.data() + string ( ".longitude" ), "0.0" );
            this->device_list_.push_back ( a_dev );
        }
    }
}

void
vca_manager::populate_vca_list()
{
    BOOST_FOREACH ( ptree::value_type& section, *global::config() )
    {
        if ( section.first.find ( "vca-" ) == 0 )
        {
            auto a_vca = make_shared< vca_info >();
            a_vca->name = section.first.data();
            a_vca->analysis = global::config()->get (
                section.first.data() + string ( ".analysis" ), "loitering" );
            a_vca->params = global::config()->get (
                section.first.data() + string ( ".params" ), "-b -nomotiondetection" );
            string device_name = global::config()->get (
                section.first.data() + string ( ".device" ), "noname" );

            BOOST_FOREACH ( auto& device, device_list_ )
            {
                if ( device->name.compare ( device_name ) == 0 )
                {
                    a_vca->device = device;
                    break;
                }
            }

            this->vca_list_.push_back ( a_vca );

            LOG ( INFO ) << "vca: " << a_vca->name
                        << " device: " << a_vca->device->name;
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

bool
vca_manager::is_device_registered ( device_info const& device )
{
    LOG ( INFO ) << "Checking device existence: " << device.name;

    string ip ( global::config()->get ( "core.device_management_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.device_management_port", 10889 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );

    transport->open();

    vector< DeviceDetails > devices;
    dmsc.listDevices ( devices, "0" );

    LOG ( INFO ) << devices.size() << " devices in database.";

    auto found = false;
    BOOST_FOREACH ( auto a_device, devices )
    {
        LOG ( INFO ) << "Device: " << a_device.name;
        if ( device.name.compare ( a_device.name ) == 0)
        {
            found = true;
            break;
        }
    }

    transport->close();
    return found;
}

void
vca_manager::update_device ( device_info const& device )
{
    LOG ( INFO ) << "Updating device: " << device.name;

    string ip ( global::config()->get ( "core.device_management_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.device_management_port", 10889 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );

    transport->open();

    vector< DeviceDetails > stored_devs;
    dmsc.listDevices ( stored_devs, "0" );

    DeviceDetails stored_dev;
    bool device_found = false;
    BOOST_FOREACH ( auto const& dev, stored_devs )
    {
        if ( dev.name.compare ( device.name ) == 0 )
        {
            device_found = true;
            stored_dev = dev;
            break;
        }
    }

    if ( !device_found )
        throw "device not found";

    vector< DeviceModel > stored_models;
    dmsc.listModels ( stored_models );
    bool model_found = false;
    string model_id;
    BOOST_FOREACH ( auto m, stored_models )
    {
        if ( m.name.compare ( device.model ) == 0 )
        {
            model_id = m.id;
            model_found = true;
            break;
        }
    }

    if ( !model_found )
        throw "device model not found";

    stored_dev.modelId = model_id;
    stored_dev.host = device.host;
    stored_dev.port = device.port;
    stored_dev.login = device.username;
    stored_dev.password = device.password;
    stored_dev.address = device.location;
    stored_dev.lat = device.latitude;
    stored_dev.lng = device.longitude;
    stored_dev.cloudRecordingEnabled = "1";

    if ( !dmsc.updateDevice ( stored_dev ) )
        throw "could not update device";

    transport->close();
}

void
vca_manager::register_device ( device_info const& device )
{
    LOG ( INFO ) << "Registering device: " << device.name;

    string ip ( global::config()->get ( "core.device_management_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.device_management_port", 10889 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );

    transport->open();

    // Get model id of the new device.
    vector< DeviceModel > models;
    string model_id;
    dmsc.listModels ( models );
    bool model_found = false;
    BOOST_FOREACH ( auto a_model, models )
    {
        if ( a_model.name.compare ( device.model ) == 0 )
        {
            model_id = a_model.id;
            model_found = true;
            break;
        }
    }

    if ( !model_found )
        throw "device model not found";

    // Fill out new device details.
    DeviceDetails new_device;

    new_device.id = "null";
    new_device.key = "null";
    new_device.name = device.name;
    new_device.modelId = model_id;
    new_device.host = device.host;
    new_device.port = device.port;
    new_device.login = device.username;
    new_device.password = device.password;
    new_device.address = device.location;
    new_device.lat = device.latitude;
    new_device.lng = device.longitude;
    new_device.accountId = "2";
    new_device.cloudRecordingEnabled = "1";
    new_device.functionalityId = "1";
    new_device.statusId = "1";
    new_device.alertFlag = "0";
    new_device.snapshotRecordingEnabled = "0";
    new_device.snapshotRecordingInterval = "0";
    new_device.alive = "1";
    new_device.currentPositionId = "1";

    string retval;

    dmsc.addDevice ( retval, new_device );
    LOG ( INFO ) << "Core returned: " << retval;

    transport->close();
}

void
vca_manager::handle_register_devices()
{
    LOG ( INFO ) << "Registering devices...";

    BOOST_FOREACH ( auto device, device_list_ )
    {
        try
        {
            if ( !is_device_registered ( *device ) )
                register_device ( *device );
            else
                update_device ( *device );
        }
        catch ( std::exception const& e )
        {
            LOG ( INFO ) << e.what();
        }
    }

    auto evt = Q_NEW ( gevt, EVT_DEVICES_REGISTERED );
    this->postFIFO ( evt );
}

string
vca_manager::get_device_id ( device_info const& device )
{
    string ip ( global::config()->get ( "core.device_management_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.device_management_port", 10889 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );

    transport->open();

    vector< DeviceDetails > devices;
    dmsc.listDevices ( devices, "0" );

    string device_id ( "-1" );

    BOOST_FOREACH ( auto const& a_device, devices )
    {
        if ( device.name.compare ( a_device.name ) == 0)
        {
            device_id = a_device.id;
            break;
        }
    }

    transport->close();

    return device_id;
}


string
vca_manager::get_mjpeg_url ( device_info const& device )
{
    LOG ( INFO ) << "Getting mjpeg stream url for device: " << device.name;
    auto device_id = get_device_id ( device );

    string ip ( global::config()->get ( "core.stream_controller_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.stream_controller_port", 10600 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    StreamControlServiceClient scsc ( protocol );

    transport->open();

    vector< string > urls;
    vector< string > clients { "localhost", "127.0.0.1" };

    string session_id = device_id;
    string stream_type ( "http/mjpeg" );

    scsc.beginStreamSession (
        urls, session_id, 10000,
        stream_type, clients, device_id,
        "0", "", "" );

    string url ( "" );
    if ( urls.size() > 0 )
        url = urls[0];

    transport->close();

    return url;
}

QP::QActive* create_vca_manager()
{
    return new vca_manager();
}

}
