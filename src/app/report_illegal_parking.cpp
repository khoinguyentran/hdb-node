#include "report_illegal_parking.hpp"
#include "common.hpp"
#include "global.hpp"

#include <glog/logging.h>

#include <sstream>

namespace app
{

using namespace boost::posix_time;
using std::ostringstream;

void
report_illegal_parking::start()
{
    LOG ( INFO ) << "illegal_parking [" << name_ << "]: starting...";

    working_dir_ = global::config()->get ( "vca.data_dir", "data" ) /  name_;
    create_directories ( working_dir_ );

    pre_event_period_ = seconds ( parameters_.get ( "pre_event_period", 300 ) );
    post_event_period_ = seconds ( parameters_.get ( "post_event_period", 300 ) );
    clip_length_ = seconds ( parameters_.get ( "clip_length", 900 ) );
    event_check_interval_ = seconds ( 10 );
    event_check_timer_.expires_from_now ( event_check_interval_ );
    event_check_timer_.async_wait ( boost::bind ( &report_illegal_parking::process_event, this ) );
    io_service_thread_ = boost::thread ( boost::bind ( &asio::io_service::run, &io_service_ ) );
}

void
report_illegal_parking::stop()
{

}

string
report_illegal_parking::desc() const
{
    ostringstream description;
    description << "illegal-parking: " << name_ << " "
                << loiter_->desc();
    return description.str();
}

void
report_illegal_parking::process_event()
{
    // Check if there are unprocessed events in the database.
    auto sql = global::get_database_handle();
    sqlite3_stmt* stmt;
    ostringstream query;

    query << "SELECT * FROM events WHERE processed=0"
        << " and reporter='" << loiter_->name() << "'";
    auto query_cstr = query.str().c_str();
    sqlite3_prepare_v2(sql, query_cstr, strlen ( query_cstr ) + 1, &stmt, NULL );

    int retval = sqlite3_step(stmt);
    if (retval == SQLITE_ROW)
    {
        auto evt_id = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 0 ) ) );
        auto evt_ts = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 1 ) ) );
        auto evt_type = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 2 ) ) );
        auto evt_reporter = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 3 ) ) );
        auto evt_device = string ( reinterpret_cast< const char *> ( sqlite3_column_text ( stmt, 4 ) ) );

        auto evt_time = common::parse_utc_string ( evt_ts );
        auto evt_start_time = evt_time - pre_event_period_;
        auto evt_completion_time = evt_time + post_event_period_;
        auto now = microsec_clock::universal_time();
        if ( now >= evt_completion_time + clip_length_ )
        {
            LOG ( INFO ) << "Getting snapshots for event: " << evt_id
                        << " start: " << common::get_utc_string ( evt_start_time )
                        << " end: " << common::get_utc_string ( evt_completion_time );
            auto snapshots = loiter_->list_snapshots_between ( evt_device, evt_start_time, evt_completion_time );

            // Create directory for event.
            auto evt_dir = working_dir_ / common::get_simple_utc_string ( evt_time ) / evt_device;
            create_directories ( evt_dir );

            // Copy the snapshots to event directory.
            BOOST_FOREACH ( auto const& s, snapshots )
            {
                auto to = evt_dir / s.filename();
                copy_file ( s, to, copy_option::overwrite_if_exists );

                // Queue the snapshot for upload.
                ostringstream insert_stmt;
                insert_stmt << "INSERT INTO uploads VALUES (NULL,"
                    << "'" << common::get_utc_string ( evt_time ) << "',"
                    << "'" << evt_type << "',"
                    << "'" << evt_reporter << "',"
                    << "'" << evt_device << "',"
                    << "'" << to.string() << "', 0)";
                int error = sqlite3_exec ( sql, insert_stmt.str().c_str(), 0, 0, 0 );
                if ( error )
                {
                    LOG ( INFO ) << "Could not insert upload in database.";
                }
            }

            LOG ( INFO ) << "Getting videos for event: " << evt_id
                        << " start: " << common::get_utc_string ( evt_start_time )
                        << " end: " << common::get_utc_string ( evt_completion_time );
            auto videos = loiter_->list_videos_between ( "",
                    evt_start_time - clip_length_, evt_completion_time + clip_length_ );

            // Copy the videos to event directory.
            BOOST_FOREACH ( auto const& v, videos )
            {
                auto to = evt_dir / v.filename();
                copy_file ( v, to, copy_option::overwrite_if_exists );

                // Queue the snapshot for upload.
                ostringstream insert_stmt;
                insert_stmt << "INSERT INTO uploads VALUES (NULL,"
                    << "'" << common::get_utc_string ( evt_time ) << "',"
                    << "'" << evt_type << "',"
                    << "'" << evt_reporter << "',"
                    << "'" << evt_device << "',"
                    << "'" << to.string() << "', 0)";
                int error = sqlite3_exec ( sql, insert_stmt.str().c_str(), 0, 0, 0 );
                if ( error )
                {
                    LOG ( INFO ) << "Could not insert upload in database.";
                }
            }

            // Update the status of the event.
            ostringstream update_stmt;
            update_stmt << "UPDATE events set processed=1 where id=" << evt_id;
            int error = sqlite3_exec ( sql, update_stmt.str().c_str(), 0, 0, 0 );
            if ( error )
            {
                LOG ( INFO ) << "Could not update event status in database.";
            }
        }
    }
    sqlite3_finalize(stmt);

    event_check_timer_.expires_from_now ( event_check_interval_ );
    event_check_timer_.async_wait ( boost::bind ( &report_illegal_parking::process_event, this ) );
}

}
