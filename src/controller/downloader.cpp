#include "fsm.hpp"
#include "global.hpp"

#include <glog/logging.h>
#include <curl/curl.h>
#include <json/json.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <fstream>
#include <queue>
#include <string>
#include <vector>

Q_DEFINE_THIS_FILE

namespace controller
{
using namespace boost::filesystem;

using boost::regex;
using boost::regex_search;
using boost::shared_ptr;
using boost::smatch;

using std::ifstream;
using std::ofstream;
using std::endl;
using std::queue;
using std::string;
using std::vector;

class downloader : public QP::QActive
{
private:
    QP::QTimeEvt timeout_event_;
    QP::QActive* controller_;
    
public:
    downloader();
    
private:
    void login();
    static size_t login_callback(void *, size_t, size_t, void *);
    void get_manifest();
    static size_t get_manifest_callback(void *, size_t, size_t, void *);
    void clear_reply();
    void get_file();
    static size_t get_file_callback(void *, size_t, size_t, void *);

private:
    CURL* curl_;
    string reply_;
    bool reply_finished_;
    size_t content_length_;    
    bool in_header_;
    boost::thread worker_thread_;
    string manifest_;
    string filename_;
    string version_;
    
protected:
    static QP::QState initial(downloader * const, QP::QEvt const * const);
    static QP::QState active(downloader * const, QP::QEvt const * const);
    static QP::QState fetch_manifest(downloader * const, QP::QEvt const * const);
    static QP::QState fetch_file(downloader * const, QP::QEvt const * const);
};

downloader::downloader()
    : QActive(Q_STATE_CAST(&downloader::initial)),
      timeout_event_(EVT_TIMEOUT)
{
}

QP::QState
downloader::initial(downloader * const me, QP::QEvt const * const e)
{
    me->controller_ = global::get_default_controller();       
    
    // Initialize curl instance.
    me->curl_ = curl_easy_init();
    if (!me->curl_)
    {
        LOG(ERROR) << "a CURL instance could not be instantiated.";
    }
    return Q_TRAN(&downloader::active);
}

// active /
QP::QState
downloader::active(downloader * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        LOG(INFO) << "Downloader started.";
        status = Q_HANDLED();
        break;
    case EVT_FETCH_MANIFEST:
        status = Q_TRAN(&downloader::fetch_manifest);
        break;
    case EVT_FETCH_FILE:
    {
        auto evt = static_cast< gevt const * const >(e);
        me->filename_ = evt->args.get< string >("filename");
        me->version_ = evt->args.get< string >("version");   
        status = Q_TRAN(&downloader::fetch_file);
        break;
    }
    default:        
        status = Q_SUPER(&QHsm::top);
        break;
    }
    
    return status;
}

// active / fetch_manifest
QP::QState
downloader::fetch_manifest(downloader * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        LOG(INFO) << "Fetching manifest...";
        me->worker_thread_ = boost::thread(
            boost::bind(&downloader::login, me));
        
        status = Q_HANDLED();
        break;           
    case EVT_LOGGED_IN:
        me->worker_thread_ = boost::thread(
            boost::bind(&downloader::get_manifest, me));        
        status = Q_TRAN(&downloader::active);
        break;
    case EVT_LOGIN_FAILED:
        status = Q_TRAN(&downloader::active);
        break;        
    default:
        status = Q_SUPER(&downloader::active);
        break;
    }
    
    return status;
}

// active / fetch_file
QP::QState
downloader::fetch_file(downloader * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        me->login();
        status = Q_HANDLED();
        break;           
    case EVT_LOGGED_IN:
        me->worker_thread_ = boost::thread(
            boost::bind(&downloader::get_file, me));        
        status = Q_TRAN(&downloader::active);
        break;
    case EVT_LOGIN_FAILED:
        status = Q_TRAN(&downloader::active);
        break;        
    default:
        status = Q_SUPER(&downloader::active);
        break;
    }
    
    return status;
}


void
downloader::login()
{
    LOG(INFO) << "Logging in...";
    
    clear_reply();
    
    CURLcode curl_error_;

    curl_easy_setopt(curl_, CURLOPT_URL,
            global::config()->get("login.url", "url=http://localhost:9001/login").c_str());    
    curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, login_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)this);
    string login_fields = string("username=")
        + global::config()->get("login.username", "admin") + "&"
        + "password=" + global::config()->get("login.password", "admin");
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, login_fields.c_str());

    curl_error_ = curl_easy_perform(curl_);

    if (curl_error_ != CURLE_OK)
    {
        LOG(INFO) << curl_easy_strerror(curl_error_);

        auto evt = Q_NEW(gevt, EVT_LOGIN_FAILED);
        this->postFIFO(evt);
    }
}

size_t
downloader::login_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto realsize = size * nmemb;
    auto me = (downloader*)userdata;

    if (!me->reply_finished_)
    {
        me->reply_.assign((char*)ptr, realsize);

        boost::regex e("PLAY_SESSION=(.+);");
        string::const_iterator start = me->reply_.begin();
        string::const_iterator end = me->reply_.end();
        boost::match_results<string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        if (regex_search(start, end, what, e, flags))
        {
            LOG(INFO) << "Session = " << what[1];
            auto evt = Q_NEW(gevt, EVT_LOGGED_IN);
            me->postFIFO(evt);
        }
    }

    return realsize;    
}

void
downloader::get_manifest()
{
    LOG(INFO) << "Getting manifest...";
    
    clear_reply();
    
    CURLcode curl_error_;
    
    string current_version = global::config()->get< string >("info.package_version");

    curl_easy_setopt(curl_, CURLOPT_URL,
        global::config()->get("update.url",
            "url=http://localhost:9001/softwareupdate/checkforupdate").c_str());    
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, get_manifest_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)this);
    string params = "node-id=" + global::config()->get("node.id", "testid") + "&"
        + "node-os=" + global::config()->get("info.os_version", "FreeBSD-amd64") + "&"
        + "current-version=" + current_version;
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, params.c_str());

    curl_error_ = curl_easy_perform(curl_);

    if (curl_error_ != CURLE_OK)
    {
        auto evt = Q_NEW(gevt, EVT_MANIFEST_UNAVAILABLE);
        this->postFIFO(evt);
    }
}

size_t
downloader::get_manifest_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto realsize = size * nmemb;

    auto me = (downloader*)userdata;

    if (!me->reply_finished_)
    {
        me->reply_.assign((char*)ptr, realsize);        

        boost::match_results<string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        try
        {
            if (regex_search(me->reply_.cbegin(), me->reply_.cend(), what,
                        boost::regex("Content-Length: ([[:digit:]]+)"), flags))
            {
                LOG(INFO) << "Content-Length = " << what[1];
                me->content_length_ = boost::lexical_cast<size_t>(what[1]);
            }
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << e.what();
            auto evt = Q_NEW(gevt, EVT_MANIFEST_UNAVAILABLE);
            me->controller_->postFIFO(evt);
        }

        if (me->content_length_ != 0)
        {
            Json::Value root;
            Json::Reader reader;
            bool parse_ok = reader.parse(me->reply_, root);

            if (parse_ok)
            {
                string result = root.get("result", "error").asString();
                if (result.compare("ok") == 0)
                {
                    auto evt = Q_NEW(gevt, EVT_MANIFEST_AVAILABLE);
                    evt->args.put("procedures", root.get("procedures", "").asString());
                    evt->args.put("new-version", root.get("new-version", "0").asString());
                    me->controller_->postFIFO(evt);
                }
                else
                {
                    auto evt = Q_NEW(gevt, EVT_MANIFEST_UNAVAILABLE);
                    me->controller_->postFIFO(evt);                 
                }
                me->reply_finished_ = true;
                me->reply_.clear();
            }
        }
    }

    return realsize;
}

void
downloader::clear_reply()
{
    reply_.clear();
    reply_finished_ = false;
    content_length_ = 0;
    in_header_ = true;
}
void
downloader::get_file()
{
    LOG(INFO) << "Fetching file " << filename_;
    clear_reply();
    
    CURLcode curl_error_;

    curl_easy_setopt(curl_, CURLOPT_URL,
        global::config()->get("download.url", "url=http://localhost:9001/softwareupdate/getfile").c_str());    
    curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);    
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, get_file_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)this);
    string params = string("node-id=test-id&")
        + "node-os=" + global::config()->get("info.os_version", "FreeBSD-amd64") + "&"
        + "version=" + version_ + "&"
        + "filename=" + filename_;
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, params.c_str());

    curl_error_ = curl_easy_perform(curl_);
    
    if (curl_error_ != CURLE_OK)
    {
        auto evt = Q_NEW(gevt, EVT_FILE_FETCH_FAILED);
        this->controller_->postFIFO(evt);
    }
}

size_t
downloader::get_file_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    downloader* me = (downloader*)userdata;
    size_t realsize = size * nmemb;

    if (!me->reply_finished_)
    {
        if (me->in_header_)
        {
            me->reply_.assign((char*)ptr, realsize);
            
            if (me->reply_.compare("\r\n") == 0)
            {
                try
                {
                    path base(global::config()->get("download.dir", "temp-download"));
                    path full = operator/(base, path(me->filename_));
                    LOG(INFO) << "Creating tree " << full.parent_path().string();
                    create_directories(full.parent_path());
                    LOG(INFO) << "Creating file " << full.string();
                    remove(full);
                    ofstream ofs(full.string(), ofstream::binary);
                    ofs.close();
                }
                catch (std::exception& e)
                {
                    LOG(WARNING) << "Error saving file " << me->filename_ << " " << e.what();
                    auto evt = Q_NEW(gevt, EVT_FILE_FETCH_FAILED);
                    me->controller_->postFIFO(evt);
                    me->reply_finished_ = true;
                }

                me->in_header_ = false;
            }
            else if (me->reply_.find("Content-Type: application/json;") != string::npos)
            {
                auto evt = Q_NEW(gevt, EVT_FILE_FETCH_FAILED);
                me->controller_->postFIFO(evt);
                me->reply_finished_ = true;
            }
            else if (me->reply_.find("Content-Length") != string::npos)
            {
                boost::match_results<string::const_iterator> what;
                boost::match_flag_type flags = boost::match_default;

                try
                {
                    if (regex_search(me->reply_.cbegin(), me->reply_.cend(), what,
                            boost::regex("Content-Length: ([[:digit:]]+)"), flags))
                    {
                        LOG(INFO) << "Content-Length = " << what[1];
                        me->content_length_ = boost::lexical_cast<size_t>(what[1]);
                    }
                }
                catch (std::exception& e)
                {
                    LOG(WARNING) << e.what();
                    auto evt = Q_NEW(gevt, EVT_FILE_FETCH_FAILED);
                    me->controller_->postFIFO(evt);
                    me->reply_finished_ = true;
                }
            }
        }
        else
        {
            path base(global::config()->get("download.dir", "temp-download"));
            path full = operator/(base, me->filename_);
            /*G(INFO) << "Writing " << realsize << " bytes to " << full.string();*/
            ofstream ofs(full.string(), ofstream::binary | ofstream::app);
            ofs.write((const char*)ptr, realsize);
            me->content_length_ -= realsize;
            if (me->content_length_ == 0)
            {
                me->reply_finished_ = true;
                ofs.close();
                auto evt = Q_NEW(gevt, EVT_FILE_FETCHED);
                evt->args.put("filename", me->filename_);
                me->controller_->postFIFO(evt);              
            }
        }
    }

    return realsize;    
}
    
QP::QActive* create_downloader()
{
    return new downloader();
}

}
