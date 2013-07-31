#include "fsm.hpp"
#include "global.hpp"

#include <qp_port.h>
#include <curl/curl.h>
#include <glog/logging.h>
#include <json/json.h>
#include <sqlite3.h>

#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <sstream>
#include <string>

namespace app
{

using boost::regex_search;
using boost::smatch;
using std::ostringstream;
using std::string;

class uploader : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
    QP::QTimeEvt vca_event_upload_reminder_;
    QP::QTimeEvt event_upload_reminder_;
    QP::QActive* controller_;
    QP::QActive* vca_manager_;

public:
    uploader();
    void clear_reply();
    void login();
    static size_t login_callback ( void*, size_t, size_t, void* );
    void upload_event ();
    static size_t upload_event_callback ( void*, size_t, size_t, void* );

private:
    CURL* curl_;
    string reply_;
    bool reply_finished_;
    size_t content_length_;
    bool in_header_;
    bool logged_in_;
    bool uploaded_;
    ptree event_args_;
    boost::thread worker_thread_;

protected:
    static QP::QState initial ( uploader* const, QP::QEvt const* const );
    static QP::QState active ( uploader* const, QP::QEvt const* const );
    static QP::QState idle ( uploader* const, QP::QEvt const* const );
    static QP::QState upload_vca_event ( uploader* const, QP::QEvt const* const );

};

// Constructor.
uploader::uploader()
    : QActive ( Q_STATE_CAST ( &uploader::initial ) ),
      timeout_ ( EVT_TIMEOUT ), vca_event_upload_reminder_ ( EVT_VCA_EVENT_UPLOAD_REMINDER ),
      event_upload_reminder_ ( EVT_EVENT_UPLOAD_REMINDER )
{
}

// initial
QP::QState uploader::initial ( uploader* const me, QP::QEvt const* const
                               e )
{
    me->controller_ = global::get_default_controller();
    me->vca_manager_ = global::get_default_vca_manager();

    me->curl_ = curl_easy_init();
    if ( !me->curl_ )
    {
        LOG ( ERROR ) << "a CURL instance could not be instantiated.";
    }

    return Q_TRAN ( &uploader::idle );
}

// active /
QP::QState uploader::active ( uploader* const me, QP::QEvt const* const e )
{
    QP::QState status;

    switch ( e->sig )
    {
    case Q_ENTRY_SIG:
    {
        LOG ( INFO ) << "Uploader started.";

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
QP::QState uploader::idle ( uploader* const me, QP::QEvt const* const e )
{
    QP::QState status;

    switch ( e->sig )
    {
    case Q_ENTRY_SIG:
    {
        auto remind_interval = SECONDS (
            global::config()->get ( "upload.remind_interval", 1 ) );   
        me->vca_event_upload_reminder_.postIn ( me, remind_interval );
        me->event_upload_reminder_.postIn ( me, remind_interval );
        status = Q_HANDLED();
        break;
    }
    
    case EVT_VCA_EVENT_UPLOAD_REMINDER:
    {
        // Check if there are yet-to-be-uploaded events in the database.
        auto sql = global::get_database_handle();
        sqlite3_stmt* stmt;
        
        char const * query = "SELECT * FROM events WHERE uploaded=0";
        sqlite3_prepare_v2(sql, query, strlen(query) + 1, &stmt, NULL );
        
        int retval = sqlite3_step(stmt);
        if (retval == SQLITE_ROW)
        {
            auto evt = Q_NEW(gevt, EVT_UPLOAD_VCA_EVENT);
            evt->args.put(
                "timestamp",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 1)))
            );
            evt->args.put(
                "description",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 2)))
            );
            evt->args.put(
                "snapshot_path",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 3)))
            );
            
            me->postFIFO(evt);            
        }
        
        sqlite3_finalize(stmt);
        
        status = Q_HANDLED();
        break;
    }    

    case EVT_UPLOAD_VCA_EVENT:
    {
        auto evt = static_cast< gevt const* const > ( e );
        LOG ( INFO ) << "Uploading event: "
                     << evt->args.get< string > ( "timestamp" ) << ": "
                     << evt->args.get< string > ( "description" ) << ": "
                     << evt->args.get< string > ( "snapshot_path" );

        me->event_args_.clear();
        me->event_args_ = evt->args;
        status = Q_TRAN ( &uploader::upload_vca_event );
        break;
    }
    
    case Q_EXIT_SIG:
        me->vca_event_upload_reminder_.disarm();
        me->event_upload_reminder_.disarm();
        status = Q_HANDLED();
        break;

    default:
        status = Q_SUPER ( &uploader::active );
        break;
    }

    return status;
}

// active / upload_vca_event
QP::QState uploader::upload_vca_event ( uploader* const me, QP::QEvt const* const e )
{
    QP::QState status;

    switch ( e->sig )
    {
    case Q_ENTRY_SIG:
    {
        me->worker_thread_ = boost::thread (
            boost::bind ( &uploader::login, me ) );
        
        status = Q_HANDLED();
        break;
    }
    
    case EVT_LOGGED_IN:
        me->worker_thread_ = boost::thread (
            boost::bind ( &uploader::upload_event , me ) );

        status = Q_HANDLED();
        break;
                
    case EVT_LOGIN_FAILED:
        status = Q_TRAN ( &uploader::idle );
        break;

    case EVT_VCA_EVENT_UPLOAD_FAILED:
    {        
        status = Q_TRAN ( &uploader::idle );
        break;
    }

    case EVT_VCA_EVENT_UPLOADED:
    {
        // Update event status to uploaded in database.
        auto sql = global::get_database_handle();
        ostringstream stmt;
        int error;
        stmt << "UPDATE events set uploaded=1 where timestamp="
        << "'" << me->event_args_.get< string > ( "timestamp" ) << "'";
        
        
        error = sqlite3_exec(sql, stmt.str().c_str(), 0, 0, 0);
        if (error)
        {
            LOG(INFO) << "Could not update event status in database.";
        }
        
        status = Q_TRAN ( &uploader::idle );
        break;
    }

    default:
        status = Q_SUPER ( &uploader::active );
        break;
    }

    return status;
}

size_t
uploader::login_callback ( void* ptr, size_t size, size_t nmemb, void* userdata )
{
    auto realsize = size * nmemb;
    auto me = ( uploader* ) userdata;

    if ( !me->reply_finished_ )
    {
        me->reply_.assign ( ( char* ) ptr, realsize );

        boost::regex e ( "PLAY_SESSION=(.+);" );
        string::const_iterator start = me->reply_.begin();
        string::const_iterator end = me->reply_.end();
        boost::match_results<string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        if ( regex_search ( start, end, what, e, flags ) )
        {
            LOG ( INFO ) << "Session = " << what[1];            
            me->logged_in_ = true;
            me->reply_finished_ = true;            
        }
    }

    return realsize;
}

void
uploader::login()
{
    CURLcode curl_error_;

    logged_in_ = false;
    clear_reply();

    LOG ( INFO ) << "Logging in...";

    curl_easy_setopt ( curl_, CURLOPT_URL,
                       global::config()->get ( "login.url", "missing" ).c_str() );
    curl_easy_setopt ( curl_, CURLOPT_VERBOSE, 0L );
    curl_easy_setopt ( curl_, CURLOPT_HEADER, 1L );
    curl_easy_setopt ( curl_, CURLOPT_COOKIEFILE, "" );
    curl_easy_setopt ( curl_, CURLOPT_COOKIESESSION, 1L );
    curl_easy_setopt ( curl_, CURLOPT_WRITEFUNCTION, login_callback );
    curl_easy_setopt ( curl_, CURLOPT_WRITEDATA, ( void* ) this );
    curl_easy_setopt ( curl_, CURLOPT_FORBID_REUSE, 1L );
    string login_fields = string ( "username=" )
                          + global::config()->get ( "login.username", "missing" ) + "&"
                          + "password=" + global::config()->get ( "login.password", "missing" );
    curl_easy_setopt ( curl_, CURLOPT_POSTFIELDS, login_fields.c_str() );

    curl_error_ = curl_easy_perform ( curl_ );

    if ( curl_error_ != CURLE_OK || !logged_in_ )
    {
        LOG ( INFO ) << curl_easy_strerror ( curl_error_ );
        auto evt = Q_NEW ( gevt, EVT_LOGIN_FAILED );
        this->postFIFO ( evt );
    }
    
    if ( logged_in_ )
    {
        auto evt = Q_NEW ( gevt, EVT_LOGGED_IN );
        this->postFIFO ( evt );
    }
}

size_t
uploader::upload_event_callback ( void* ptr, size_t size, size_t nmemb, void* userdata )
{
    auto realsize = size * nmemb;
    auto me = ( uploader* ) userdata;

    if ( !me->reply_finished_ )
    {
        if ( realsize == 0 )
        {
            me->reply_finished_ = true;
            me->uploaded_ = false;
        }
        else
        {
            me->reply_.assign ( ( char* ) ptr, realsize );

            LOG ( INFO ) << me->reply_;

            Json::Value root;
            Json::Reader reader;
            bool parse_ok = reader.parse ( me->reply_, root );
            if ( parse_ok )
            {
                string result ( root.get ( "result", "error" ).asString() );
                if ( result.compare ( "ok" ) == 0 )
                {
                    me->uploaded_ = true;
                }
                else
                {
                    me->uploaded_ = false;
                }
                me->reply_finished_ = true;
            }
        }

    }

    return realsize;
}


void
uploader::upload_event ()
{
    uploaded_ = false;
    this->clear_reply();
    auto timestamp = this->event_args_.get< string > ( "timestamp" );
    auto description = this->event_args_.get< string > ( "description" );
    auto snapshot_path = this->event_args_.get< string > ( "snapshot_path" );

    LOG ( INFO ) << "Uploading event " << description;

    CURLcode curl_error_;
    struct curl_httppost* formpost_ = NULL;
    struct curl_httppost* lastptr_ = NULL;
    
    curl_formadd ( &formpost_, &lastptr_,
                   CURLFORM_COPYNAME, "attachment",
                   CURLFORM_FILE, snapshot_path.c_str(),
                   CURLFORM_END );

    curl_formadd ( &formpost_, &lastptr_,
                   CURLFORM_COPYNAME, "event",
                   CURLFORM_COPYCONTENTS, description.c_str(),
                   CURLFORM_END );

    curl_easy_setopt ( this->curl_, CURLOPT_URL,
                       global::config()->get ( "upload.url", "missing" ).c_str() );

    curl_easy_setopt ( this->curl_, CURLOPT_HEADER, 0L );

    curl_easy_setopt ( this->curl_, CURLOPT_WRITEFUNCTION, upload_event_callback );
    curl_easy_setopt ( this->curl_, CURLOPT_WRITEDATA, this );
    curl_easy_setopt ( this->curl_, CURLOPT_POST, 1L );
    curl_easy_setopt ( this->curl_, CURLOPT_HTTPPOST, formpost_ );

    curl_error_ = curl_easy_perform ( curl_ );

    if ( curl_error_ != CURLE_OK || !uploaded_ )
    {
        auto evt = Q_NEW ( gevt, EVT_VCA_EVENT_UPLOAD_FAILED );
        this->postFIFO ( evt );
    }
    
    if (uploaded_)
    {
        auto evt = Q_NEW ( gevt, EVT_VCA_EVENT_UPLOADED );
        this->postFIFO ( evt );
    }
    
    curl_formfree ( formpost_ );
}

void
uploader::clear_reply()
{
    reply_.clear();
    reply_finished_ = false;
    content_length_ = 0;
    in_header_ = true;
}


QP::QActive* create_uploader()
{
    return new uploader();
}

}
