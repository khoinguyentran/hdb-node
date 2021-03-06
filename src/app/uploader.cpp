#include "common.hpp"
#include "fsm.hpp"
#include "global.hpp"

#include <qp_port.h>
#include <curl/curl.h>
#include <glog/logging.h>
#include <json/json.h>
#include <sqlite3.h>

#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace app
{

using namespace boost::filesystem;
using namespace boost::posix_time;
using boost::regex_search;
using boost::smatch;
using std::ostringstream;
using std::string;
using std::vector;

class uploader : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
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
    static QP::QState uploading_event ( uploader* const, QP::QEvt const* const );

};

// Constructor.
uploader::uploader()
    : QActive ( Q_STATE_CAST ( &uploader::initial ) ),
      timeout_ ( EVT_TIMEOUT ),
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
        me->event_upload_reminder_.postEvery ( me, remind_interval );
        status = Q_HANDLED();
        break;
    }

    case EVT_EVENT_UPLOAD_REMINDER:
    {
        // Check if there are yet-to-be-uploaded events in the database.
        auto sql = global::get_database_handle();
        sqlite3_stmt* stmt;

        char const * query = "SELECT * FROM uploads WHERE uploaded=0";
        sqlite3_prepare_v2(sql, query, strlen(query) + 1, &stmt, NULL );

        int retval = sqlite3_step(stmt);
        if (retval == SQLITE_ROW)
        {
            auto evt = Q_NEW(gevt, EVT_UPLOAD_EVENT);
            evt->args.put(
                "id",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 0)))
            );
            evt->args.put(
                "timestamp",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 1)))
            );
            evt->args.put(
                "type",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 2)))
            );
            evt->args.put(
                "reporter",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 3)))
            );
            evt->args.put(
                "device",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 4)))
            );
            evt->args.put(
                "upload_file",
                string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 5)))
            );

            me->postFIFO(evt);
        }

        sqlite3_finalize(stmt);

        status = Q_HANDLED();
        break;
    }

    case EVT_UPLOAD_EVENT:
    {
        auto evt = static_cast< gevt const* const > ( e );
        LOG ( INFO ) << "Uploading event: "
                     << evt->args.get< string > ( "id" ) << ": "
                     << evt->args.get< string > ( "timestamp" ) << ": "
                     << evt->args.get< string > ( "type" ) << ": "
                     << evt->args.get< string > ( "reporter" ) << ": "
                     << evt->args.get< string > ( "device" );

        me->event_args_.clear();
        me->event_args_ = evt->args;
        status = Q_TRAN ( &uploader::uploading_event );
        break;
    }

    case Q_EXIT_SIG:
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
QP::QState uploader::uploading_event ( uploader* const me, QP::QEvt const* const e )
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

    case EVT_EVENT_UPLOAD_FAILED:
    {
        status = Q_TRAN ( &uploader::idle );
        break;
    }

    case EVT_EVENT_UPLOADED:
    {
        // Update event status to uploaded in database.
        auto sql = global::get_database_handle();
        ostringstream stmt;
        int error;

        string id = me->event_args_.get< string > ( "id" );
        string type = me->event_args_.get< string > ( "type" );

        stmt << "UPDATE uploads set uploaded=1 where id=" << id;

        error = sqlite3_exec(sql, stmt.str().c_str(), 0, 0, 0);
        if (error)
        {
            LOG(INFO) << "Could not update upload status in database.";
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
    auto type = this->event_args_.get< string > ( "type" );
    auto reporter  = this->event_args_.get< string > ( "reporter" );
    auto timestamp = this->event_args_.get< string > ( "timestamp" );
    auto device = this->event_args_.get< string > ( "device" );
    auto site = global::config()->get("node.site", "invalid-site");
    auto node_name = global::config()->get("node.name", "invalid-node-name");
    auto file_path = this->event_args_.get< string > ( "upload_file" );

    CURLcode curl_error_;
    struct curl_httppost* formpost_ = NULL;
    struct curl_httppost* lastptr_ = NULL;

    if ( type.compare ( "loitering" ) == 0 )
    {
        curl_formadd ( &formpost_, &lastptr_,
                    CURLFORM_COPYNAME, "attachment",
                    CURLFORM_FILE, file_path.c_str(),
                    CURLFORM_END );
    }
    else if ( type.compare ( "health" ) == 0 )
    {
        curl_formadd ( &formpost_, &lastptr_,
                    CURLFORM_COPYNAME, "status",
                    CURLFORM_COPYCONTENTS, file_path.c_str(),
                    CURLFORM_END );
    }

    curl_formadd ( &formpost_, &lastptr_,
                   CURLFORM_COPYNAME, "type",
                   CURLFORM_COPYCONTENTS, type.c_str(),
                   CURLFORM_END );
    curl_formadd ( &formpost_, &lastptr_,
                   CURLFORM_COPYNAME, "site",
                   CURLFORM_COPYCONTENTS, site.c_str(),
                   CURLFORM_END );
    curl_formadd ( &formpost_, &lastptr_,
                    CURLFORM_COPYNAME, "nodeName",
                    CURLFORM_COPYCONTENTS, node_name.c_str(),
                    CURLFORM_END );
    curl_formadd ( &formpost_, &lastptr_,
                    CURLFORM_COPYNAME, "device",
                    CURLFORM_COPYCONTENTS, device.c_str(),
                    CURLFORM_END );
    curl_formadd ( &formpost_, &lastptr_,
                   CURLFORM_COPYNAME, "timestamp",
                   CURLFORM_COPYCONTENTS, timestamp.c_str(),
                   CURLFORM_END );

    string upload_url;
    if ( type.compare ( "loitering" ) == 0)
        upload_url = global::config()->get ( "upload.url", "missing" );
    else if ( type.compare ( "health" ) == 0 )
        upload_url = global::config()->get ( "report.url", "missing" );

    curl_easy_setopt ( this->curl_, CURLOPT_URL, upload_url.c_str() );
    curl_easy_setopt ( this->curl_, CURLOPT_HEADER, 0L );
    curl_easy_setopt ( this->curl_, CURLOPT_WRITEFUNCTION, upload_event_callback );
    curl_easy_setopt ( this->curl_, CURLOPT_WRITEDATA, this );
    curl_easy_setopt ( this->curl_, CURLOPT_POST, 1L );
    curl_easy_setopt ( this->curl_, CURLOPT_HTTPPOST, formpost_ );

    curl_error_ = curl_easy_perform ( curl_ );

    if ( curl_error_ != CURLE_OK || !uploaded_ )
    {
        LOG ( INFO ) << curl_easy_strerror ( curl_error_ );
        auto evt = Q_NEW ( gevt, EVT_EVENT_UPLOAD_FAILED );
        this->postFIFO ( evt );
    }

    if (uploaded_)
    {
        auto evt = Q_NEW ( gevt, EVT_EVENT_UPLOADED );
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
