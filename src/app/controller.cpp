#include "fsm.hpp"
#include "common.hpp"
#include "global.hpp"

#include <glog/logging.h>

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <qp_port.h>

namespace app
{
namespace asio = boost::asio;

using asio::ip::tcp;
using boost::regex;
using std::string;
    
class controller : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
    QP::QActive* uploader_;
    QP::QActive* vca_manager_;    
    
public:
    controller();
    void listen_for_command();
    void process_command(string const &);
    void run_io_service();
    
private:
    asio::io_service io_service_;
    boost::thread io_service_thread_;
    boost::thread command_listener_thread_;    
    tcp::socket controll_socket_;
    
protected:
    static QP::QState initial(controller * const, QP::QEvt const * const);
    static QP::QState active(controller * const, QP::QEvt const * const);    
};

// Constructor.
controller::controller()
    : QActive(Q_STATE_CAST(&controller::initial)),
      timeout_(EVT_TIMEOUT), controll_socket_(io_service_)
{
}

// initial
QP::QState controller::initial(controller * const me, QP::QEvt const * const e)
{
    me->uploader_ = global::get_default_uploader();    
    me->vca_manager_ = global::get_default_vca_manager();
    return Q_TRAN(&controller::active);
}

// active /
QP::QState controller::active(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        LOG(INFO) << "Controller started.";
        me->io_service_thread_ = boost::thread(
            boost::bind(&controller::run_io_service, me));
        me->command_listener_thread_ = boost::thread(
            boost::bind(&controller::listen_for_command, me));
        auto evt = Q_NEW(gevt, EVT_POWERED_ON);
        me->postFIFO(evt);
        status = Q_HANDLED();
        break;
    }
    case EVT_POWERED_ON:        
        status = Q_HANDLED();
        break;
    case EVT_SHUTDOWN:        
        std::terminate();
        status = Q_HANDLED();
        break;
    default:
        status = Q_SUPER(&QHsm::top);
        break;
    }
    
    return status;
}

void
controller::process_command(string const& cmd)
{
    if (cmd.find("shutdown") == 0)
    {        
        string reply("done\n");
        asio::write(controll_socket_, asio::buffer(reply));
        auto evt = Q_NEW(gevt, EVT_SHUTDOWN);
        this->postFIFO(evt);
    }
    else
    {
        LOG(ERROR) << "Unknown command: " << cmd;
    }
}

void
controller::listen_for_command()
{
    LOG(INFO) << "Listening for commands...";
    boost::system::error_code error;
    asio::streambuf buffer;
        
    try
    {   
        uint16_t port = global::config()->get("controller.port", 3103);
        tcp::acceptor acceptor(this->io_service_,
                               tcp::endpoint(tcp::v4(), port));   
        
        while (true)
        {   
            acceptor.accept(controll_socket_);
            while (true)
            {
                asio::read_until(controll_socket_, buffer, regex("\n"), error);
                if (error)
                    break;
                
                std::istream is(&buffer);
                string line;
                std::getline(is, line);
                process_command(line);
            }
            controll_socket_.close();
        }
    }
    catch (std::exception& e)
    {
        LOG(ERROR) << e.what();
        
    }
}

void controller::run_io_service()
{
    LOG(INFO) << "IO service started.";
    try
    {
        io_service_.run();
    }
    catch (std::exception& e)
    {
        LOG(ERROR) << e.what();
    }
}

QP::QActive* create_controller()
{
    return new controller();
}

}