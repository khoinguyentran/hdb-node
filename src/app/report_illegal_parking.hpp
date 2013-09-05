#ifndef REPORT_ILLEGAL_PARKING_HPP
#define REPORT_ILLEGAL_PARKING_HPP

#include "loitering.hpp"
#include "report.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace app
{

namespace asio = boost::asio;
using boost::shared_ptr;

class report_illegal_parking : public report
{
public:
    report_illegal_parking ( string const& name )
        : report ( name ), event_check_timer_ ( io_service_ ) {}
    ~report_illegal_parking() {}
    void start() override;
    void stop() override;

public:
    string desc() const override;
    void loiter ( shared_ptr< loitering > loiter ) { loiter_ = loiter; }

private:
    void process_event();

private:
    shared_ptr< loitering > loiter_;
    asio::io_service io_service_;
    boost::thread io_service_thread_;
    asio::deadline_timer event_check_timer_;
    time_duration event_check_interval_;
    time_duration pre_event_period_;
    time_duration post_event_period_;
    time_duration clip_length_;
};

}

#endif
