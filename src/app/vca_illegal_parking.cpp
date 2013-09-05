#include "common.hpp"
#include "global.hpp"
#include"vca_illegal_parking.hpp"

#include <glog/logging.h>
#include <sqlite3.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <sstream>

namespace app
{

using namespace boost::posix_time;
using boost::posix_time::microsec_clock;
using std::ostringstream;

string
vca_illegal_parking::desc() const
{
    ostringstream description;
    description << "vca_illegal_parking: " << name_
                << " loiter: " << loiter_->name();
    return description.str();
}

void
vca_illegal_parking::init()
{
    LOG ( INFO ) << "vca_illegal_parking[" << name_ << "]: initializing...";

    working_dir_ = operator/( global::config()->get ( "vca.data_dir", "data" ), name_ );
    create_directories ( working_dir_ );

    event_queue_.reset( new squeue< shared_ptr< ptree > >() );
    loiter_->subscribe ( event_queue_ );
}

void
vca_illegal_parking::start()
{
    init();
    LOG ( INFO ) << "vca_illegal_parking[" << name_ << "]: starting...";
    loiter_->start();
    main_thread_ = boost::thread ( boost::bind ( &vca_illegal_parking::register_event, this ) );
    worker_thread_ = boost::thread ( boost::bind ( &vca_illegal_parking::fillout_event, this ) );
}

void
vca_illegal_parking::stop()
{
    LOG ( INFO ) << "vca_illegal_parking[" << name_ << "]: stopping...";
}

void
vca_illegal_parking::restart()
{
    LOG ( INFO ) << "vca_illegal_parking[" << name_ << "]: restarting...";
}

void
vca_illegal_parking::register_event()
{
    try
    {
        while ( true )
        {
            auto evt = event_queue_->pop();

            LOG ( INFO ) << "register event type: " << evt->get< string > ( "type" )
                        << " at: " << evt->get< string > ( "timestamp" );

            // Insert event into database.
            auto sql = global::get_database_handle();
            int error;

            ostringstream stmt;
            stmt << "INSERT INTO events VALUES(NULL, "
                << "'" << evt->get< string > ( "timestamp" ) << "'" << ","
                << "'" << evt->get< string > ( "type" ) << "'" << ","
                << "'" << name_ << "'" << ","
                << "'" << evt->get< string > ( "description" ) << "'" << ", "
                << "0" << ", "
                << "''" << ", "
                << "0" << ")";

            error = sqlite3_exec ( sql, stmt.str().c_str(), 0, 0, 0 );
            if ( error )
            {
                LOG ( ERROR ) << "Could not register new event with database.";
            }
        }
    }
    catch ( std::exception const& e )
    {
        LOG ( INFO ) << e.what();
    }
}

void
vca_illegal_parking::process_event()
{
    try
    {
        while ( true )
        {
            // Check if there are yet-to-be-fillout_event events in the database.
            auto sql = global::get_database_handle();
            sqlite3_stmt* stmt;
            ostringstream query;

            query << "SELECT * FROM events WHERE snapshots_fetched=0"
                  << " and producer='" << name_ << "'";
            auto query_cstr = query.str().c_str();
            sqlite3_prepare_v2(sql, query_cstr, strlen ( query_cstr ) + 1, &stmt, NULL );

            string evt_ts;
            string evt_id;
            int retval = sqlite3_step(stmt);
            if (retval == SQLITE_ROW)
            {
                evt_id = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 0 ) ) );
                evt_ts = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 1 ) ) );
                sqlite3_finalize(stmt);
            }
            else
            {
                boost::this_thread::sleep ( seconds ( 5 ) );
                continue;
            }

            auto evt_time = common::parse_utc_string ( evt_ts );

            boost::this_thread::sleep ( seconds ( 10 ) );
            LOG ( INFO ) << "filling out event id: " << evt_id
                        << " at: " << evt_ts;

            // Create directory for event.
            path evt_dir = operator/ ( working_dir_, common::get_simple_utc_string ( evt_time ) );
            create_directories ( evt_dir );

            auto pre_event_period = parameters_.get ( "pre_event_period", 300 );
            auto post_event_period = parameters_.get ( "post_event_period", 300 );
            auto start_time = evt_time - seconds ( pre_event_period );
            auto end_time = evt_time + seconds ( post_event_period );

            ptime now ( microsec_clock::universal_time() );
            auto dt = now - end_time;
            if ( dt <= seconds ( 0 ) )
            {
                boost::this_thread::sleep ( seconds ( -dt.seconds() ) );
            }

            // Copy snapshots to event directory.
            auto images = loiter_->list_snapshots_between ( start_time, end_time );

            BOOST_FOREACH ( auto const& image, images )
            {
                path to = operator/ ( evt_dir, image.filename() );
                copy_file ( image, to, copy_option::overwrite_if_exists );

            }
        }
    }
    catch ( std::exception const& e )
    {
        LOG ( INFO ) << e.what();
    }
}

}
