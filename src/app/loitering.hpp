#ifndef LOITERING_HPP
#define LOITERING_HPP

#include "analysis.hpp"
#include "ip_camera.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <string>

namespace app
{

namespace asio = boost::asio;
using namespace boost::filesystem;
using namespace boost::posix_time;
using boost::shared_ptr;
using std::string;

class loitering : public analysis
{
public:
    loitering ( string const& name )
        : analysis ( name ), snapshot_timer_ ( io_service_ )
    {}
    ~loitering() {}

public:
    string desc() const override;
    void start() override;
    void stop() override;
    shared_ptr< ip_camera > camera() const { return camera_; }
    void camera ( shared_ptr< ip_camera > value ) { camera_ = value; }
    vector< path > list_snapshots_between ( string const &, ptime const &, ptime const & );
    vector< path > list_videos_between ( string const &, ptime const &, ptime const & );

private:
    void process();
    void snapshot();
    void cleanup();
    void fetch_file ( string const &, path const & );
    static size_t fetch_file_callback ( void *, size_t, size_t, void * );

private:
    shared_ptr< ip_camera > camera_;
    path working_dir_;
    boost::thread main_thread_;
    boost::thread io_service_thread_;
    asio::io_service io_service_;
    asio::deadline_timer snapshot_timer_;
    path snapshot_dir_;
    size_t snapshot_interval_;
    string snapshot_url_;
};

}

#endif
