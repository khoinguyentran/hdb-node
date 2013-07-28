#include "fsm.hpp"
#include "common.hpp"
#include "global.hpp"

#include <qp_port.h>
#include <glog/logging.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <string>
#include <sstream>

namespace app
{
    
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using std::ostringstream;
using std::string;
    
class vca_manager : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
    QP::QTimeEvt vca_event_timer_;
    QP::QActive* controller_;
    QP::QActive* uploader_;    
    
public:
    vca_manager();
    void log_vca_event(gevt *);
    
private:
    
private:
    
protected:
    static QP::QState initial(vca_manager * const, QP::QEvt const * const);
    static QP::QState active(vca_manager * const, QP::QEvt const * const);   
    
};

// Constructor.
vca_manager::vca_manager()
    : QActive(Q_STATE_CAST(&vca_manager::initial)),
    timeout_(EVT_TIMEOUT), vca_event_timer_(EVT_GENERATE_VCA_EVENT)
{
}

// initial
QP::QState vca_manager::initial(vca_manager * const me, QP::QEvt const * const 
e)
{
    me->controller_ = global::get_default_controller();    
    me->uploader_ = global::get_default_uploader();
    return Q_TRAN(&vca_manager::active);
}

// active /
QP::QState vca_manager::active(vca_manager * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG(INFO) << "VCA Manager started.";            
            
            me->vca_event_timer_.postEvery(me, SECONDS(3));
            
            status = Q_HANDLED();
            break;
        }
        
        case EVT_GENERATE_VCA_EVENT:
        {
            LOG(INFO) << "Generating vca event...";
            
            auto evt = Q_NEW(gevt, EVT_VCA_EVENT);
            evt->args.put("description", "violation event at site001");
            ptime timestamp(microsec_clock::universal_time());
            evt->args.put("timestamp", common::get_utc_string(timestamp));
            evt->args.put("snapshot_path", "snapshots/vca.jpg");
            
            LOG(INFO) << evt->args.get< string >("timestamp") << ": "
                      << evt->args.get< string >("description") << ": "
                      << evt->args.get< string >("snapshot_path");  
            
            me->log_vca_event(evt);
            me->postFIFO(evt);
                        
            status = Q_HANDLED();
            break;
        }
        
        case EVT_VCA_EVENT:
        {
            auto evt = Q_NEW(gevt, EVT_UPLOAD_VCA_EVENT);
            evt->args = static_cast< gevt const * const >(e)->args;
            me->uploader_->postFIFO(evt);
            
            status = Q_HANDLED();
            break;
        }
        
        default:
            status = Q_SUPER(&QHsm::top);
            break;
    }
    
    return status;
}

void vca_manager::log_vca_event(gevt* e)
{
    auto sql = global::get_database_handle();
    int error;
    
    ostringstream stmt;
    stmt << "INSERT INTO vca_events VALUES(NULL, "
        << "'" << e->args.get< string >("timestamp") << "'" << ","
        << "'" << e->args.get< string >("description") << "'" << ", "        
        << "'" << e->args.get< string >("snapshot_path") << "'" << ", "
        << "0" << ")";
    
    error = sqlite3_exec(sql, stmt.str().c_str(), 0, 0, 0);
    if (error)
    {
        LOG(INFO) << "Could not register new event with database.";
    }
}
    
QP::QActive* create_vca_manager()
{
    return new vca_manager();
}

}