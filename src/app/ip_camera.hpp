#ifndef IP_CAMERA_HPP
#define IP_CAMERA_HPP

#include "device.hpp"

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <vector>

namespace app
{

namespace asio = boost::asio;
using namespace boost::filesystem;
using namespace boost::posix_time;
using boost::asio::ip::tcp;
using std::vector;

class device_model_not_found : public std::runtime_error
{
public:
    device_model_not_found ( string const & name )
        : runtime_error ( "device_model_not_found" ),
        name_ ( name ) {}
    string name_;
};

class device_update_with_core_failed : public std::runtime_error
{
public:
    device_update_with_core_failed ( string const & name )
        : runtime_error ( "device_update_with_core_failed" ),
        name_ ( name ) {}
    string name_;
};

class ip_camera : public device
{
public:
    ip_camera ( string const& name ) : device ( name ),
        online_check_timer_ ( io_service_ ),
        online_check_socket_ ( io_service_ ) {}
    ~ip_camera() {}

    string desc() const override;
    void start() override;
    void stop() override;

public:
    void host ( string const& value ) { host_ = value; }
    void port ( string const& value ) { port_ = value; }
    void username ( string const& value ) { username_ = value; }
    void password ( string const& value ) { password_ = value; }
    void online_check_interval ( size_t value ) { online_check_interval_ = value; }
    string mjpeg_url() const { return mjpeg_url_; }
    string jpeg_url() const { return jpeg_url_; }
    vector< string > list_video_urls_between ( ptime const &, ptime const &);

private:
    void online_check();
    bool is_registered_with_core();
    void register_with_core();
    void update_with_core();
    string get_device_id();
    string get_stream_url( string const & );

protected:
    string host_;
    string port_;
    string username_;
    string password_;
    string mjpeg_url_;
    string jpeg_url_;
    size_t online_check_interval_;

    asio::io_service io_service_;
    boost::thread io_service_thread_;
    asio::deadline_timer online_check_timer_;
    tcp::socket online_check_socket_;

};

}

#endif
