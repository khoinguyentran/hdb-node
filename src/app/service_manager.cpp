#include "service_manager.hpp"
#include "vca_illegal_parking.hpp"

#include <glog/logging.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

namespace app
{

using boost::make_shared;

void
service_manager::load ( ptree const& config )
{
    LOG ( INFO ) << "Loading vca configurations...";

    BOOST_FOREACH ( ptree::value_type const& section, config )
    {
        string name ( section.first.data() );
        if ( name.find ( "vca-" ) != 0 )
            continue;

        string type ( config.get ( name + string ( ".type" ), "" ) );
        if ( type.empty() )
            continue;

        shared_ptr< vca > a_vca;
        if ( type.compare ( "illegal parking" ) == 0 )
        {
            auto illegal_parking = make_shared< vca_illegal_parking > ( section.first.data() );
            illegal_parking->type ( type );
            string loiter_name = config.get ( section.first.data() + string ( ".loiter" ), "" );

            shared_ptr< loitering > loiter =
                boost::static_pointer_cast< loitering > ( analysismgr_->find_by_name ( loiter_name ) );
            if ( !loiter )
            {
                LOG ( INFO ) << "Could not find loitering analysis with name: " << loiter_name;
                continue;
            }

            ptree params;
            BOOST_FOREACH ( ptree::value_type const& c, config.get_child ( name ) )
            {
                params.put ( c.first.data(), c.second.data() );
            }

            loiter->parameters ( params );
            illegal_parking->loiter ( loiter );
            a_vca = illegal_parking;
        }
        else
            continue;

        try
        {
            add ( a_vca );
            LOG ( INFO ) << "Added: " << a_vca->desc();
        }
        catch ( std::exception const& e )
        {
            LOG ( INFO ) << e.what();
        }
    }
}

void
service_manager::start_all()
{
    LOG ( INFO ) << "service_manager: starting all services...";

    BOOST_FOREACH ( auto& v, vcas_ )
    {
        try
        {
            v->start();
        }
        catch ( std::exception const& e )
        {
            LOG ( INFO ) << e.what();
        }
    }
}

void
service_manager::stop_all()
{

}

void
service_manager::resume_all()
{

}

vector< shared_ptr< vca > >
service_manager::find_by_type ( string const& vca_type )
{
    vector< shared_ptr< vca > > results;

    BOOST_FOREACH ( auto const& a_vca, vcas_ )
    {
        if ( a_vca->type().compare ( vca_type ) == 0 )
            results.push_back ( a_vca );
    }

    return results;
}

void
service_manager::add ( shared_ptr< vca > a_vca )
{
    BOOST_FOREACH ( auto const& v, vcas_ )
    {
        if ( v->name().compare ( a_vca->name () ) == 0 )
            throw service_name_duplicated ( v->name() );
    }

    vcas_.push_back ( a_vca );
}

}
