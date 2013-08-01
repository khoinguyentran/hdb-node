#include "fsm.hpp"
#include "common.hpp"
#include "global.hpp"

#include <glog/logging.h>
#include <openssl/sha.h>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

Q_DEFINE_THIS_FILE

namespace controller
{
using namespace boost::filesystem;
namespace asio = boost::asio;

using boost::regex;
using boost::regex_match;
using boost::regex_search;
using boost::shared_ptr;
using boost::smatch;
using boost::tuple;
using boost::asio::ip::tcp;

using std::ifstream;
using std::ofstream;
using std::endl;
using std::getline;
using std::istream;
using std::queue;
using std::string;
using std::vector;

class controller : public QP::QActive
{
private:
    QP::QTimeEvt timeout_;
    QP::QTimeEvt update_reminder_;
    QP::QActive* downloader_;
    
public:
    controller();
    
private:
    //void clear_lists();    
    //void log_command(string const &);
    //void execute_command();
    bool is_update_interrupted();
    void resume_update_commands();
    void populate_update_commands(string const &);
    void execute_command(string const &);
    void copy_file(shared_ptr< path >, shared_ptr< path >);
    void app_communicate(shared_ptr< string > request);
    void checksum(shared_ptr< path >, shared_ptr< string >);
    void execute_program(shared_ptr< path >);
    void log_cmd(string const &);
    
private:
    queue< string > cmds_;    
    path cmd_file_path_;
    path cmd_log_path_;
    string new_version_;
    asio::io_service io_service_;
    boost::thread worker_thread_;
    boost::thread io_thread_;
    
protected:
    static QP::QState initial(controller * const, QP::QEvt const * const);
    static QP::QState active(controller * const, QP::QEvt const * const);
    static QP::QState recovery(controller * const, QP::QEvt const * const);
    static QP::QState update(controller * const, QP::QEvt const * const);
    static QP::QState check_for_update(controller * const, QP::QEvt const * const);
    static QP::QState fetch_update(controller * const, QP::QEvt const * const);
    static QP::QState pre_update(controller * const, QP::QEvt const * const);
    static QP::QState do_update(controller * const, QP::QEvt const * const);
    static QP::QState post_update(controller * const, QP::QEvt const * const);
    static QP::QState execute_update_commands(controller * const, QP::QEvt const * const);
    static QP::QState remote_control(controller * const, QP::QEvt const * const);
};

// Constructor.
controller::controller()
    : QActive(Q_STATE_CAST(&controller::initial)),
      timeout_(EVT_TIMEOUT), update_reminder_(EVT_CHECK_FOR_UPDATE)
{
}

// initial
QP::QState controller::initial(controller * const me, QP::QEvt const * const e)
{
    me->downloader_ = global::get_default_downloader();      
    me->cmd_file_path_ = path(global::config()->get("command_file", "update.cmd"));    
    me->cmd_log_path_ = path(global::config()->get("command_log", "update.cmd.log"));
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
        auto evt = Q_NEW(gevt, EVT_POWERED_ON);
        me->postFIFO(evt);
        status = Q_HANDLED();
        break;
    }
    case EVT_POWERED_ON:
        status = Q_TRAN(&controller::recovery);
        break;
    case EVT_CHECK_FOR_UPDATE:
        status = Q_TRAN(&controller::check_for_update);        
        break;
    default:
        status = Q_SUPER(&QHsm::top);
        break;
    }
    
    return status;
}

// active / recovery
QP::QState controller::recovery(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        LOG(INFO) << "Recovery started.";
        
        if (me->is_update_interrupted())
        {
            auto evt = Q_NEW(gevt, EVT_UPDATE_INTERRUPTED);
            me->postFIFO(evt);
        }
        else
        {
            auto evt = Q_NEW(gevt, EVT_CHECK_FOR_UPDATE);
            me->postFIFO(evt);
        }
        
        status = Q_HANDLED();
        break;
    }
    case EVT_UPDATE_INTERRUPTED:    
        me->resume_update_commands();
        status = Q_TRAN(&controller::execute_update_commands);
        break;    
    default:
        status = Q_SUPER(&controller::active);
        break;
    }
    
    return status;
}

// active / update
QP::QState controller::update(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:    
        status = Q_HANDLED();
        break;    
    case Q_EXIT_SIG:
        me->update_reminder_.postIn(me, SECONDS(global::config()->get("update.interval", 10)));
        remove(me->cmd_file_path_);
        remove(me->cmd_log_path_);
        status = Q_HANDLED();
        break;
    default:
        status = Q_SUPER(&controller::active);
        break;
    }
    
    return status;
}

// active / update / check_for_update
QP::QState controller::check_for_update(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:    
    {
        LOG(INFO) << "Checking for update...";
        
        auto evt = Q_NEW(gevt, EVT_FETCH_MANIFEST);        
        me->downloader_->postFIFO(evt);
        status = Q_HANDLED();
        break;    
    }
    case Q_EXIT_SIG:
    {
        me->update_reminder_.postIn(me, SECONDS(global::config()->get("update.interval", 10)));
        status = Q_HANDLED();
        break;
    }
    case EVT_MANIFEST_UNAVAILABLE:
    {
        status = Q_TRAN ( &controller::active );
        break;
    }
    case EVT_MANIFEST_AVAILABLE:
    {
        auto evt = static_cast< gevt const * const >(e);
        common::print_ptree(evt->args);
        
        string procedures(evt->args.get< string >("procedures"));
        if (procedures.empty())
        {
            LOG(INFO) << "Package is up-to-date.";
            status = Q_TRAN(&controller::active);
        }
        else
        {
            try
            {
                me->populate_update_commands(procedures);
                status = Q_TRAN(&controller::execute_update_commands);
            }
            catch (std::exception const& e)
            {
                LOG(INFO) << e.what();
                status = Q_TRAN(&controller::active);
            }
        }        
        
        break;
    }
    default:
        status = Q_SUPER(&controller::update);
        break;
    }
    
    return status;
}

// active / update / fetch_update
QP::QState controller::fetch_update(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:    
    {
        LOG(INFO) << "Fetching update...";
        status = Q_HANDLED();
        break;    
    }
    case EVT_FILE_FETCHED:
    {
        status = Q_HANDLED();        
        break;
    }
    default:
        status = Q_SUPER(&controller::update);
        break;
    }
    
    return status;
}

// active / update / execute_update_commands
QP::QState controller::execute_update_commands(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:    
    {
        LOG(INFO) << "Executing update commands...";
        auto evt = Q_NEW(gevt, EVT_EXECUTE_COMMAND);
        evt->args.put("command", me->cmds_.front());
        me->postFIFO(evt);
        status = Q_HANDLED();
        break;    
    }
    case EVT_EXECUTE_COMMAND:
    {
        auto evt = static_cast< gevt const * const >(e);
        me->execute_command(evt->args.get< string >("command"));
        status = Q_HANDLED();
        break;
    }
    case EVT_COMMAND_EXECUTED:
        me->cmds_.pop();
        if (me->cmds_.size() > 0)
        {
            auto evt = Q_NEW(gevt, EVT_EXECUTE_COMMAND);
            evt->args.put("command", me->cmds_.front());
            me->postFIFO(evt);
        }
        status = Q_HANDLED();
        break;
    case EVT_FILE_FETCHED:
    {
        auto evt = static_cast< gevt const * const >(e);
        if (me->cmds_.front().find("get") == 0
            && me->cmds_.front().find(
                evt->args.get("filename", "/")) != string::npos)        
        {            
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);            
            me->postFIFO(evt);
        }
        status = Q_HANDLED();
        break;
    }
    case EVT_UPDATE_SUCCEEDED:
    {
        LOG(INFO) << "Updated to version " << me->new_version_;
        status = Q_TRAN(&controller::active);
        break;
    }
    case Q_EXIT_SIG:
    {
        
        status = Q_HANDLED();
        break;
    }
    default:
        status = Q_SUPER(&controller::update);
        break;
    }
    
    return status;
}

// active / remote_control
QP::QState controller::remote_control(controller * const me, QP::QEvt const * const e)
{
    QP::QState status;
    
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {        
        status = Q_HANDLED();
        break;
    }
    default:
        status = Q_SUPER(&controller::active);
        break;
    }
    
    return status;
}

QP::QActive* create_controller()
{
    return new controller();
}

bool controller::is_update_interrupted()
{
    ifstream cmd_file(cmd_file_path_.string());
    string line = "";
    bool is_interrupted = false;
    
    while (getline(cmd_file, line))
    {
        cmds_.push(line);
        if (line.find("end") == 0)
            is_interrupted = true;
    }
    
    return is_interrupted;
}

void controller::resume_update_commands()
{
    string line;
    
    // Populate original commands.
    ifstream cmd_file(cmd_file_path_.string());    
    while (getline(cmd_file, line))
    {        
        cmds_.push(line);
    }
    cmd_file.close();
    
    // Populate command logs.
    queue< string > cmds_log_;
    ifstream cmd_log(cmd_log_path_.string());
    while (getline(cmd_log, line))
    {
        cmds_log_.push(line);
    }
    
    smatch what;
    line = this->cmds_.front();
    if (regex_match(line, what, regex("^(version) \"([^\"]+)\"$")))
        this->new_version_ = what[2];
    else
        throw "Version is missing from update commands.";
    
    while (cmds_log_.size() > 1)
    {
        this->cmds_.pop();
        cmds_log_.pop();
    }
    
    if (this->cmds_.front().find("restart") == 0)
        cmds_.pop();
}

void controller::populate_update_commands(string const& manifest)
{
    boost::match_results<string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    string::const_iterator start;
    string::const_iterator end;
    boost::regex add_re("^(Add|AddExecutable|Update|UpdateExecutable) \"([^\"]+)\" ([[:alnum:]]+)");
    boost::regex delete_re("^(Delete) \"([^\"]+)\"");
    boost::regex version_re("^(Version) \"([^\"]+)\"");
    boost::regex end_re("^(end)");
    
    string controller_name(global::config()->get< string >("info.module_name"));
    start = manifest.begin();
    end = manifest.end();

    this->cmds_ = queue< string >();
    
    // Parse version identifier.
    regex_search(start, end, what, version_re, flags);    
    this->new_version_ = what[2].str();
    
    // Check if manifest is valid.
    //if (!regex_search(start, end, what, end_re, flags))
    //    throw "Manifest corrupted.";
    
    // Parse commands.
    vector< string > set_exec;    
    vector< string > copy_files;
    vector< tuple< string, string > > get_files;    
    vector< string > delete_files;
    vector< string > update_controller_cmds;
    
    while (regex_search(start, end, what, add_re, flags))
    {
        string command(what[1]);
        string filepath(what[2]);
        string checksum(what[3]);        
        
        if (command.find("Executable") != string::npos)
        {
            set_exec.push_back(filepath);
        }
        if (command.find("Add") != string::npos)
        {
            get_files.push_back(tuple< string, string > (filepath, checksum));
            copy_files.push_back(filepath);
        }
        if (command.find("Update") == 0)
        {
            path p(filepath);
            if ((*p.begin()).string().compare(global::config()->get<string>("update.app_dir")) != 0)            
            {
                path p_old("");
                auto it = p.begin();
                
                while (it != p.end())
                {
                    p_old /= *it;
                    it++;
                }

                path p_new(p_old.string() + ".old");
                string module_name(global::config()->get< string >("info.module_name"));
                if (p.filename().string().compare(module_name) != 0)
                {                    
                    update_controller_cmds.insert(
                        update_controller_cmds.begin(),
                        "rename \"" + p_old.string() + "\" \""
                        + p_new.string() + "\"");
                }
                else
                {
                    update_controller_cmds.push_back(
                            "rename \"" + p_old.string() + "\" \""
                            + p_new.string() + "\"");
                }
                path p_src = operator/(path(global::config()->get< string >("download.dir")),
                        path(filepath));
                update_controller_cmds.push_back(
                        "copy \"" +  p_src.string() + "\" \""
                        + p_old.string() + "\"");
            }            
            get_files.push_back(tuple< string, string > (filepath, checksum));                        
        }         
        
        start = what[0].second;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    } // end while
    
    // Parse delete commands.
    start = manifest.begin();
    end = manifest.end();
    flags = boost::match_default;
    
    while (regex_search(start, end, what, add_re, flags))
    {
        string command(what[1]);
        string filepath(what[2]);
        
        delete_files.push_back(filepath);
        
        start = what[0].second;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }
    
    
    // Write update command file.
    ofstream cmd_file(cmd_file_path_.string());
    cmd_file << "version \"" << this->new_version_ << "\"" << endl;
    
    // Files to download.
    
    for (int i = 0; i < get_files.size(); ++i)
        cmd_file << "get \"" << get_files[i].get<0>() << "\"" << endl;
    
    // Files to checksum.
    for (int i = 0; i < get_files.size(); ++i)
        cmd_file << "check \"" << get_files[i].get<0>() << "\" \""
            << get_files[i].get<1>() << "\"" << endl;
            
    // Update commands.
    path src, dest;
    path temp_dir(global::config()->get("download.dir", "temp-download"));
    path dest_dir(".");
    
    // Set file permissions.
    for (int i = 0; i < set_exec.size(); ++i)
    {
        path p = operator/(temp_dir, set_exec[i]);
        cmd_file << "set_exec " << p << endl;
    }
    
    // Request main application to stop.
    cmd_file << "shutdown_main" << endl;    
    cmd_file << "set_exec \"pre-update.sh\"" << endl;
    cmd_file << "set_exec \"post-update.sh\"" << endl;
    cmd_file << "exec \"./pre-update.sh\"" << endl;
    
    // Copy new files.
    for (int i = 0; i < copy_files.size(); ++i)
    {
        path relative(copy_files[i]);
        src = operator/(temp_dir, relative);
        dest = operator/(dest_dir, relative);
        cmd_file << "copy \"" << src.string() << "\" "
            << "\"" << dest.string() << "\"" << endl;
    }
    
    // Controller update commands.
    for (int i = 0; i < update_controller_cmds.size(); ++i)
    {
        cmd_file << update_controller_cmds[i] << endl;
    }
    
    if (update_controller_cmds.size() > 0)
        cmd_file << "restart" << endl;
    
    // Update main software commands.
    cmd_file << "exec \"./post-update.sh\"" << endl;
    cmd_file << "end" << endl;
    cmd_file.close();
    
    // Populate command queue.
    ifstream ifs(global::config()->get("update.command_file", "update.cmd"));
    string line;
    while (!ifs.eof())
    {        
        getline(ifs, line);
        this->cmds_.push(line);
    }
    ifs.close();
}

void
controller::execute_command(string const& cmd)
{
    LOG(INFO) << cmd;
    
    log_cmd(cmd);
    
    smatch what;
    boost::regex get_re("^(get) \"([^\"]+)\"$");          
    boost::regex check_re("^(check) \"([^\"]+)\" \"([^\"]+)\"$");
    boost::regex set_exec_re("^(set_exec) \"([^\"]+)\"$");
    boost::regex exec_re("^(exec) \"([^\"]+)\"$");
    boost::regex rename_re("^(rename) \"([^\"]+)\" \"([^\"]+)\"$");
    boost::regex copy_re("^(copy) \"([^\"]+)\" \"([^\"]+)\"$");
    
    if (regex_match(cmd, what, get_re))
    {
        auto evt = Q_NEW(gevt, EVT_FETCH_FILE);
        evt->args.put("filename", what[2]);
        evt->args.put("version", new_version_);            
        this->downloader_->postFIFO(evt);          
    }     
    else if (regex_match(cmd, what, check_re))
    {
        shared_ptr< path > file(new path(what[2]));
        shared_ptr< string > csum(new string(what[3]));
        
        worker_thread_ = boost::thread(
            boost::bind(&controller::checksum, this, file, csum));
    }
    else if (regex_match(cmd, what, set_exec_re))
    {
        path p(what[2]);
        
        if (status(p).permissions() & owner_exe)
        {
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
            this->postFIFO(evt);
        }
        else
        {
            try
            {
                permissions(p, add_perms | owner_exe);
                auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
                this->postFIFO(evt);
            }
            catch (std::exception& e)
            {
                LOG(INFO) << e.what();
                auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTION_FAILED);
                this->postFIFO(evt);
            }
        }
    }
    else if (regex_match(cmd, what, exec_re))
    {
        shared_ptr< path > script(new path(what[2]));
        worker_thread_ = boost::thread(
            boost::bind(&controller::execute_program, this, script));
    }          
    else if (regex_match(cmd, what, rename_re))
    {
        path src(what[2]);
        path dest(what[3]);
        try
        {
            rename(src, dest);
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
            this->postFIFO(evt);
        }
        catch (std::exception& e)
        {
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTION_FAILED);
            this->postFIFO(evt);
        }                
    }
    else if (regex_match(cmd, what, copy_re))
    {
        shared_ptr< path > src(new path(what[2]));
        shared_ptr< path > dest(new path(what[3]));
        worker_thread_ = boost::thread(
            boost::bind(&controller::copy_file, this, src, dest));
    }
    else if (cmd.find("restart") == 0)
    {
        std::terminate();
    }
    else if (cmd.find("end") == 0)
    {
        auto evt = Q_NEW(gevt, EVT_UPDATE_SUCCEEDED);
        this->postFIFO(evt);
    }   
    else if (cmd.find("version") == 0)
    {
        auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
        this->postFIFO(evt);
    }
    else if (cmd.find("shutdown_main") == 0)
    {
        shared_ptr< string > request(new string("shutdown\n"));
        worker_thread_ = boost::thread(
            boost::bind(&controller::app_communicate, this, request));
    }
    else
    {
        auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTION_FAILED);
        this->postFIFO(evt);
    }
}

void
controller::copy_file(shared_ptr< path > src, shared_ptr<path> dest)
{
    boost::filesystem::copy_file(*src, *dest, copy_option::overwrite_if_exists);
    auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
    this->postFIFO(evt);
}

void controller::app_communicate(shared_ptr< string > request)
{
    try
    {
        tcp::socket s(this->io_service_);
        tcp::resolver resolver(this->io_service_);
        tcp::resolver::query query(tcp::v4(),
            "localhost", global::config()->get("app.port", "3103"));
        tcp::resolver::iterator iterator = resolver.resolve(query);
        
        asio::connect(s, iterator);
        
        LOG(INFO) << "Sending request " << *request << " to app.";
        
        asio::write(s, asio::buffer(*request));
        
        asio::streambuf b;            
        asio::read_until(s, b, "\n");
        istream is(&b);
        string line;
        std::getline(is, line);

        if (line.find("done") == 0)
        {
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
            this->postFIFO(evt);
        }
        else
        {
            auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTION_FAILED);
            this->postFIFO(evt);
        }            
    }
    catch (std::exception& e)
    {
        LOG(INFO) << e.what();
        auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
        this->postFIFO(evt);
    }
}

void
controller::checksum(shared_ptr< path > file_path, shared_ptr< string > csum)
{
    auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
    this->postFIFO(evt);
}

void
controller::execute_program(shared_ptr< path > script_path)
{
    system(script_path->string().c_str());
    auto evt = Q_NEW(gevt, EVT_COMMAND_EXECUTED);
    this->postFIFO(evt);
}

void
controller::log_cmd(string const& cmd)
{
    ofstream ofs;
    ofs.exceptions(ofstream::failbit | ofstream::badbit);

    try
    {
        ofs.open(global::config()->get("update.command_log", "update.cmd.log"),
            ofstream::app);            
        ofs << cmd << endl;
        ofs.close();
    }
    catch (std::exception const& e)
    {
        LOG(WARNING) << e.what();
    }
}

}
