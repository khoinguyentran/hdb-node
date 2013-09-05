#include "report_manager.hpp"
#include "report_illegal_parking.hpp"

#include <glog/logging.h>

#include <boost/make_shared.hpp>

namespace app
{

using boost::make_shared;

void
report_manager::load ( ptree const& config )
{
    LOG ( INFO ) << "Loading report configurations...";

    BOOST_FOREACH ( ptree::value_type const& section, config )
    {
        string name ( section.first.data() );
        if ( name.find ( "report-" ) != 0 )
            continue;

        string type ( config.get ( name + string ( ".type" ), "" ) );
        if ( type.empty() )
            continue;

        shared_ptr< report > a_report;
        if ( type.compare ( "illegal-parking" ) == 0 )
        {
            auto illegal_parking = make_shared< report_illegal_parking > ( name );
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

            illegal_parking->parameters ( params );
            illegal_parking->loiter ( loiter );
            a_report = illegal_parking;
        }
        else
            continue;

        try
        {
            add ( a_report );
            LOG ( INFO ) << "Added: " << a_report->desc();
        }
        catch ( std::exception const& e )
        {
            LOG ( INFO ) << e.what();
        }
    }
}

void
report_manager::start_all()
{
    BOOST_FOREACH ( auto const& r, reports_ )
    {
        try
        {
            r->start();
        }
        catch ( std::exception const& e )
        {
            LOG ( WARNING ) << e.what();
        }
    }
}

void
report_manager::stop_all()
{

}

void
report_manager::add ( shared_ptr< report > a_report )
{

    BOOST_FOREACH ( auto const& r, reports_ )
    {
        if ( r->name().compare ( a_report->name () ) == 0 )
            throw report_name_duplicated ( r->name() );
    }

    reports_.push_back ( a_report );
}

}
