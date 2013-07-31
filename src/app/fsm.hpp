#ifndef FSM_HPP
#define FSM_HPP

#include "bsp.hpp"

#include <qp_port.h>

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

#include <iostream>

namespace app
{
using boost::shared_ptr;
using boost::property_tree::ptree;
    
enum signal
{
    // Public signals.
    SHUTDOWN_SIG = QP::Q_USER_SIG,
    MAX_PUB_SIG,
    
    // Controller events.
    EVT_POWERED_ON,
    EVT_UPDATE_INTERRUPTED,
    EVT_CHECK_FOR_UPDATE,
    EVT_REMOTE_CONTROL,
    EVT_EXECUTE_COMMAND,
    EVT_COMMAND_EXECUTED,
    EVT_COMMAND_EXECUTION_FAILED,
    EVT_UPDATE_SUCCEEDED,
    EVT_SHUTDOWN,
    
    // Uploader events.    
    EVT_VCA_EVENT_UPLOAD_REMINDER,
    EVT_EVENT_UPLOAD_REMINDER,
    EVT_UPLOAD_VCA_EVENT,
    EVT_VCA_EVENT_UPLOADED,
    EVT_VCA_EVENT_UPLOAD_FAILED,
    
    // Downloader events.
    EVT_LOGGED_IN,
    EVT_LOGIN_FAILED,
    EVT_FETCH_MANIFEST,
    EVT_MANIFEST_AVAILABLE,
    EVT_MANIFEST_UNAVAILABLE,
    EVT_FETCH_FILE,   
    EVT_FILE_FETCHED,
    EVT_FILE_FETCH_FAILED,
    
    // VCA Manager events
    EVT_GENERATE_VCA_EVENT,
    EVT_VCA_EVENT,
    EVT_VCA_STOPPED,
    
    // Common events.
    EVT_TIMEOUT
};

// Event prototypes.
class gevt : public QP::QEvt
{
public:    
    gevt ( QP::QSignal const s ) : QP::QEvt ( s ) {}
    ptree args;
    virtual ~gevt() { args.clear(); }
};

// Component creation functions.
QP::QActive* create_controller();
QP::QActive* create_uploader();
QP::QActive* create_vca_manager();

}

#endif