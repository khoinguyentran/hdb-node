#ifndef VCA_ILLEGAL_PARKING_HPP
#define VCA_ILLEGAL_PARKING_HPP

#include "ip_camera.hpp"
#include "loitering.hpp"
#include "squeue.hpp"
#include "vca.hpp"

namespace app
{

using app::loitering;
using app::vca;

class vca_illegal_parking : public vca
{
public:
    vca_illegal_parking ( string const& name ) : vca ( name ) {}
    ~vca_illegal_parking() {}

public:
    string desc() const override;
    void init() override;
    void start() override;
    void stop() override;
    void restart() override;

public:
    void loiter ( shared_ptr< loitering > loiter ) { loiter_ = loiter; }

private:
    void register_event();
    void fillout_event();

private:
    shared_ptr< loitering > loiter_;
    shared_ptr< sequeue > event_queue_;
    boost::thread main_thread_;
    boost::thread worker_thread_;
};

}

#endif
