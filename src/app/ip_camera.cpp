#include "ip_camera.hpp"
#include "common.hpp"
#include "global.hpp"

#include "corecomm/DeviceManagementService.h"
#include "corecomm/StreamControlService.h"
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include <protocol/TBinaryProtocol.h>
#include <glog/logging.h>

#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <sstream>
#include <vector>

namespace app
{

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace com::kaisquare::core::thrift;
using namespace boost::posix_time;

using boost::shared_ptr;

using std::ostringstream;
using std::vector;

string
ip_camera::desc() const
{
    ostringstream description;
    description << "ip_camera: " << name_
                << " model: " << model_
                << " host: " << host_
                << " port: " << port_
                << " location: " << location_;
    return description.str();
}

void
ip_camera::start()
{
    LOG ( INFO ) << "ip_camera [" << name_ << "]: starting...";

    if ( !is_registered_with_core() )
        register_with_core();
    else
        update_with_core();

    mjpeg_url_ = get_stream_url ( "http/mjpeg" );
    jpeg_url_ = get_stream_url ( "http/jpeg" );

    online_check_timer_.expires_from_now ( seconds ( online_check_interval_) );
    online_check_timer_.async_wait ( boost::bind ( &ip_camera::online_check, this ) );

    io_service_thread_ = boost::thread ( boost::bind ( &asio::io_service::run, &io_service_ ) );

}

void
ip_camera::stop()
{
    LOG ( INFO ) << "ip_camera [" << name_ << "]: stopping...";
    io_service_.stop();
    io_service_thread_.join();
}

void
ip_camera::register_with_core()
{
    LOG ( INFO ) << "ip_camera [" << name_ << "]: registering self with core...";

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
        if ( model_.compare ( a_model.name ) == 0 )
        {
            model_id = a_model.id;
            model_found = true;
            break;
        }
    }

    if ( !model_found )
        throw device_model_not_found ( model_ );

    // Fill out new device details.
    DeviceDetails new_device;

    new_device.id = "null";
    new_device.key = "null";
    new_device.name = name_;
    new_device.modelId = model_id;
    new_device.host = host_;
    new_device.port = port_;
    new_device.login = username_;
    new_device.password = password_;
    new_device.address = location_;
    new_device.lat = latitude_;
    new_device.lng = longitude_;
    new_device.accountId = "2";
    new_device.cloudRecordingEnabled = "true";
    new_device.functionalityId = "1";
    new_device.statusId = "1";
    new_device.alertFlag = "0";
    new_device.snapshotRecordingEnabled = "1";
    new_device.snapshotRecordingInterval = "1";
    new_device.alive = "1";
    new_device.currentPositionId = "1";

    string retval;

    dmsc.addDevice ( retval, new_device );

    transport->close();
}

bool
ip_camera::is_registered_with_core()
{
    LOG ( INFO ) << "ip_camera [" << name_ << "]: checking existence within core...";

    string ip ( global::config()->get ( "core.device_management_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.device_management_port", 10889 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );

    transport->open();

    vector< DeviceDetails > devices;
    dmsc.listDevices ( devices, "0" );

    auto found = false;
    BOOST_FOREACH ( auto a_device, devices )
    {
        if ( name_.compare ( a_device.name ) == 0)
        {
            found = true;
            break;
        }
    }

    transport->close();
    return found;
}

void
ip_camera::update_with_core()
{
    LOG ( INFO ) << "ip_camera [" << name_ << "]: update entry with core...";

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
        if ( name_.compare ( dev.name ) == 0 )
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
        if ( model_.compare ( m.name ) == 0 )
        {
            model_id = m.id;
            model_found = true;
            break;
        }
    }

    if ( !model_found )
        throw device_model_not_found ( model_ );

    stored_dev.modelId = model_id;
    stored_dev.host = host_;
    stored_dev.port = port_;
    stored_dev.login = username_;
    stored_dev.password = password_;
    stored_dev.address = location_;
    stored_dev.lat = latitude_;
    stored_dev.lng = longitude_;
    stored_dev.cloudRecordingEnabled = "true";

    if ( !dmsc.updateDevice ( stored_dev ) )
        throw device_update_with_core_failed ( name_ );

    transport->close();
}

string
ip_camera::get_device_id()
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

    string device_id ( "" );

    BOOST_FOREACH ( auto const& a_device, devices )
    {
        if ( name_.compare ( a_device.name ) == 0)
        {
            device_id = a_device.id;
            break;
        }
    }

    transport->close();

    return device_id;
}

string
ip_camera::get_stream_url ( string const& stream_type = "http/mjpeg" )
{
    auto device_id = get_device_id();

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

void
ip_camera::online_check()
{
    LOG ( INFO ) << "Checking camera for availability...";

    tcp::resolver resolver ( io_service_ );
    auto endpoint_iterator = resolver.resolve ( { host_, port_ } );

    asio::async_connect ( online_check_socket_, endpoint_iterator,
        [ this ] ( boost::system::error_code ec, tcp::resolver::iterator connected_iterator )
        {
            if ( !ec )
            {
                LOG ( WARNING ) << "ip_camera [" << name_ << "]: is online.";
            }
            else
            {
                LOG ( WARNING ) << "ip_camera [" << name_ << "]: is offline.";
            }
            online_check_socket_.close();

            auto sql = global::get_database_handle();
            ptime now = microsec_clock::universal_time();
            ostringstream insert_stmt;
            insert_stmt << "INSERT INTO uploads VALUES (NULL,"
            << "'" << common::get_utc_string ( now ) << "',"
            << "'health',"
            << "'" << name_ << "',"
            << "'" << name_ << "',"
            << "'" << (!ec ? "online" : "offline") << "', 0)";
            int error = sqlite3_exec ( sql, insert_stmt.str().c_str(), 0, 0, 0 );
            if ( error )
            {
                LOG ( INFO ) << "Could not insert upload in database.";
            }

            online_check_timer_.expires_from_now ( seconds ( online_check_interval_ ) );
            online_check_timer_.async_wait ( boost::bind ( &ip_camera::online_check, this ) );
        }
        );
}

vector< string >
ip_camera::list_video_urls_between ( ptime const& start, ptime const& end)
{
    auto device_id = get_device_id();

    string ip ( global::config()->get ( "core.stream_controller_host", "localhost" ) );
    uint16_t port = global::config()->get ( "core.stream_controller_port", 10600 );

    shared_ptr< TSocket > socket ( new TSocket ( ip , port ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    StreamControlServiceClient scsc ( protocol );

    transport->open();

    vector< string > urls;
    vector< string > clients { "localhost", "127.0.0.1", "10.10.200.162" };

    string session_id = device_id + "1";

    scsc.beginStreamSession (
        urls, session_id, 10000,
        "rtsp/h264", clients, device_id,
        "0", common::get_ddMMyyyyHHmmss_utc_string ( start ),
        common::get_ddMMyyyyHHmmss_utc_string ( end ) );

    transport->close();

    return urls;
}

}
