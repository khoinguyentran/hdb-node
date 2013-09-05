#include "device_manager.hpp"
#include "ip_camera.hpp"

#include <glog/logging.h>

#include <boost/foreach.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <algorithm>
#include <string>

namespace app
{

using boost::make_shared;
using std::string;

void
device_manager::load ( ptree const& config )
{
    LOG ( INFO ) << "Loading device configurations...";

    BOOST_FOREACH ( ptree::value_type const& section, config )
    {
        string name ( section.first.data() );
        if ( name.find ( "device-" ) != 0 )
            continue;

        string type ( config.get ( name + string ( ".type" ), "" ) );
        if ( type.empty() )
            continue;

        shared_ptr< device > a_dev;
        if ( type.compare ( "IP Camera" ) == 0 )
        {
            auto ipcam = make_shared< ip_camera >( section.first.data() );
            ipcam->type ( type );
            ipcam->host ( config.get ( section.first.data() + string ( ".host" ), "localhost" ) );
            ipcam->model ( config.get ( section.first.data() + string ( ".model" ), "Amtk IP Camera" ) );
            ipcam->port ( config.get ( section.first.data() + string ( ".port" ), "80" ) );
            ipcam->username ( config.get ( section.first.data() + string ( ".username" ), "admin" ) );
            ipcam->password ( config.get ( section.first.data() + string ( ".password" ), "admin" ) );
            ipcam->online_check_interval ( config.get ( section.first.data() + string ( ".online_check_interval" ), 60 ) );
            ipcam->location ( config.get ( section.first.data() + string ( ".location" ), "N/A" ) );
            ipcam->latitude ( config.get ( section.first.data() + string ( ".latitude" ), "0.0" ) );
            ipcam->longitude ( config.get ( section.first.data() + string ( ".longitude" ), "0.0" ) );
            a_dev = ipcam;
        }
        else
            continue;

        this->add ( a_dev );
        LOG ( INFO ) << "Added: " << a_dev->desc();
    }
}

void
device_manager::start_all()
{
    BOOST_FOREACH ( auto& dev, devices_ )
    {
        try
        {
            dev->start();
        }
        catch ( std::exception const& e )
        {
            LOG ( WARNING ) << e.what();
        }
    }
}

void
device_manager::stop_all()
{
    BOOST_FOREACH ( auto& dev, devices_ )
    {
        try
        {
            dev->stop();
        }
        catch ( std::exception const& e )
        {
            LOG ( WARNING ) << e.what();
        }
    }
}

shared_ptr< device >
device_manager::find_by_name ( string const& dev_name )
{
    BOOST_FOREACH ( auto const& dev, devices_ )
    {
        if ( dev->name().compare ( dev_name ) == 0 )
            return dev;
    }

    return nullptr;
}

void
device_manager::add ( shared_ptr< device > a_device )
{
    BOOST_FOREACH ( auto const& dev, devices_ )
    {
        if ( dev->name().compare ( a_device->name () ) == 0 )
            throw device_name_duplicated ( dev->name() );
    }

    devices_.push_back ( a_device );
}

}
