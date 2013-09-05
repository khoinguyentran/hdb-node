#include "analysis_manager.hpp"
#include "loitering.hpp"

#include <glog/logging.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

namespace app
{

using boost::make_shared;

void
analysis_manager::load ( ptree const& config )
{
    LOG ( INFO ) << "Loading analysis configurations...";

    BOOST_FOREACH ( ptree::value_type const& section, config )
    {
        string name ( section.first.data() );
        if ( name.find ( "analysis-" ) != 0 )
            continue;

        string type ( config.get ( name + string ( ".type" ), "" ) );
        if ( type.empty() )
            continue;

        shared_ptr< analysis > an_analysis;
        if ( type.compare ( "loitering" ) == 0 )
        {
            auto loiter = make_shared< loitering >( section.first.data() );
            loiter->type ( type );
            string ipcam_name = config.get ( section.first.data() + string ( ".camera" ), "" );
            shared_ptr< ip_camera > ipcam =
                boost::static_pointer_cast< ip_camera > ( devmgr_->find_by_name ( ipcam_name ) );
            if ( !ipcam )
            {
                LOG ( INFO ) << "Could not find ip camera with name: " << ipcam_name;
                continue;
            }

            ptree params;
            BOOST_FOREACH ( ptree::value_type const& c, config.get_child ( name ) )
            {
                params.put ( c.first.data(), c.second.data() );
            }

            loiter->parameters ( params );
            loiter->camera ( ipcam );
            an_analysis = loiter;
        }
        else
            continue;

        try
        {
            add ( an_analysis );
            LOG ( INFO ) << "Added: " << an_analysis->desc();
        }
        catch ( std::exception const& e )
        {
            LOG ( INFO ) << e.what();
        }
    }
}

void
analysis_manager::start_all()
{
    BOOST_FOREACH ( auto& an_analysis, analyses_ )
    {
        try
        {
            an_analysis->start();
        }
        catch ( std::exception const& e )
        {
            LOG ( WARNING ) << e.what();
        }
    }
}

void
analysis_manager::stop_all()
{
    BOOST_FOREACH ( auto& an_analysis, analyses_ )
    {
        try
        {
            an_analysis->stop();
        }
        catch ( std::exception const& e )
        {
            LOG ( WARNING ) << e.what();
        }
    }
}

shared_ptr< analysis >
analysis_manager::find_by_name ( string const& analysis_name )
{
    BOOST_FOREACH ( auto const& an_analysis, analyses_ )
    {
        if ( an_analysis->name().compare ( analysis_name ) == 0 )
            return an_analysis;
    }

    return nullptr;
}

vector< shared_ptr< analysis > >
analysis_manager::find_by_type ( string const& analysis_type )
{
    vector< shared_ptr< analysis > > results;

    BOOST_FOREACH ( auto const& an_analysis, analyses_ )
    {
        if ( an_analysis->type().compare ( analysis_type ) == 0 )
            results.push_back ( an_analysis );
    }

    return results;
}

void
analysis_manager::add ( shared_ptr< analysis > an_analysis )
{
    BOOST_FOREACH ( auto const& a, analyses_ )
    {
        if ( a->name().compare ( an_analysis->name () ) == 0 )
            throw analysis_name_duplicated ( a->name() );
    }

    analyses_.push_back ( an_analysis );
}

}
