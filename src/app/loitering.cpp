#include "common.hpp"
#include "global.hpp"
#include "loitering.hpp"

#include <curl/curl.h>
#include <glog/logging.h>
#include <pstream.h>

#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/chrono.hpp>

#include <fstream>
#include <sstream>

namespace app
{

using namespace boost::filesystem;
using boost::make_shared;
using std::ofstream;
using std::ostringstream;

string
loitering::desc() const
{
    ostringstream description;
    description << "loitering: " << name_
                << " camera: " << camera_->name();
    return description.str();
}

void
loitering::start()
{
    LOG ( INFO ) << "loitering [" << name_ << "]: starting...";

    working_dir_ = operator/( global::config()->get ( "vca.data_dir", "data" ), name_ );
    create_directories ( working_dir_ );

    snapshot_dir_ = working_dir_ / camera_->name();
    create_directories ( snapshot_dir_ );
    snapshot_interval_ = parameters_.get ( "snapshot_interval", 60 );
    snapshot_url_ = camera_->jpeg_url();

    snapshot_timer_.expires_from_now ( seconds ( snapshot_interval_) );
    snapshot_timer_.async_wait ( boost::bind ( &loitering::snapshot, this ) );

    io_service_thread_ = boost::thread ( boost::bind ( &asio::io_service::run, &io_service_ ) );

    main_thread_ = boost::thread ( boost::bind ( &loitering::process, this ) );
}

void
loitering::stop()
{

}

vector< path >
loitering::list_snapshots_between ( string const& camera, ptime const& from, ptime const& to )
{
    vector< path > results;
    directory_iterator end_iter;

    path search_path = working_dir_ / camera;

    if ( !exists ( search_path ) || !is_directory ( search_path ) )
        return results;

    for ( directory_iterator dir_iter ( search_path ); dir_iter != end_iter; ++dir_iter )
    {
        if ( is_regular_file ( dir_iter->status() ) )
        {
            string filename = dir_iter->path().filename().string();
            auto snapshot_time = common::parse_simple_utc_string ( filename );
            if ( snapshot_time >= from && snapshot_time < to )
            {
                results.push_back ( dir_iter->path() );
            }
        }
    }

    return results;
}

vector< path >
loitering::list_videos_between ( string const& camera, ptime const& from, ptime const& to )
{
    auto video_urls = camera_->list_video_urls_between ( from, to );
    vector< path > videos;

    BOOST_FOREACH ( auto const& url, video_urls )
    {
        if ( url.find ( ".MP4" ) != string::npos )
        {
            path local_file ( url.substr ( url.length() -18 ) );
            path absolute = snapshot_dir_ / local_file;
            fetch_file ( url, absolute );
            videos.push_back ( absolute );
            LOG ( INFO ) << absolute.string();
        }
    }

    return videos;
}

void
loitering::process()
{
    LOG ( INFO ) << "loitering [" << name_ << "]: main process started.";

    string mjpeg_url = camera_->mjpeg_url();

    ostringstream cmdline;
    cmdline << "vca/vca -i '" << mjpeg_url << "' "
        << parameters_.get ( "cmd_args", "-t" );

    LOG ( INFO ) << "loitering[" << name_ << "]: " << cmdline.str();

    redi::ipstream in ( cmdline.str() );
    string line;

    ptime evt_start;
    ptime now;
    auto confirm_duration = parameters_.get ( "confirm_duration", 300 );
    bool suspected = false;

    while ( std::getline ( in, line ) )
    {
        now = microsec_clock::universal_time();
        if ( suspected )
        {
            auto dt = now - evt_start;
            if ( dt >= seconds ( confirm_duration ) )
            {
                LOG ( INFO ) << "loitering [" << name_ << "]: violation.";
                ptime timestamp = microsec_clock::universal_time();

                // Insert event into database.
                auto sql = global::get_database_handle();
                int error;

                ostringstream stmt;
                stmt << "INSERT INTO events VALUES(NULL, "
                    << "'" << common::get_utc_string ( timestamp ) << "'" << ","
                    << "'" << type_ << "'" << ","
                    << "'" << name_ << "'" << ","
                    << "'" << camera_->name() << "'" << ","
                    << "'" << line << "'" << ","
                    << "0" << ")";

                error = sqlite3_exec ( sql, stmt.str().c_str(), 0, 0, 0 );
                if ( error )
                {
                    LOG ( ERROR ) << "Could not register new event with database.";
                }
                suspected = false;
            }
        }
        else
        {
            evt_start = microsec_clock::universal_time();
            suspected = true;
        }

    }
}

void
loitering::snapshot()
{
    ptime timestamp ( microsec_clock::universal_time() );
    path local_file ( common::get_simple_utc_string ( timestamp ) + ".jpg" );
    path absolute = snapshot_dir_ / local_file;

    LOG ( INFO ) << "loitering [" << name_ << "]: fetching image to " << absolute;
    fetch_file ( snapshot_url_, absolute );

    snapshot_timer_.expires_from_now ( seconds ( snapshot_interval_) );
    snapshot_timer_.async_wait ( boost::bind ( &loitering::snapshot, this ) );
}

void
loitering::cleanup()
{
    //LOG ( INFO ) << "loitering [" << name_ << "]: cleaning up started.";

    //try
    //{
        //while ( true )
        //{

            //boost::this_thread::sleep_for ( boost::chrono::seconds ( 5 ) );
        //}
    //}
    //catch ( std::exception const& e )
    //{
        //LOG ( INFO ) << e.what();
    //}
}

void
loitering::fetch_file ( string const& url, path const& local_path )
{
    ofstream* ofs = new ofstream ( local_path.string(), ofstream::binary | ofstream::trunc );
    auto curl = curl_easy_init();
    CURLcode curl_error;

    if ( ofs != NULL && curl != NULL )
    {
        curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() );
        curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, &loitering::fetch_file_callback );
        curl_easy_setopt ( curl, CURLOPT_WRITEDATA, ofs );
        curl_error = curl_easy_perform ( curl );
        curl_easy_cleanup ( curl );
        ofs->close();
    }
}

size_t
loitering::fetch_file_callback ( void* ptr, size_t size, size_t nmemb, void* userdata )
{
    auto realsize = size * nmemb;
    auto ofs = ( ofstream* ) userdata;
    ofs->write ( ( const char* ) ptr, realsize );
    return realsize;
}

}
