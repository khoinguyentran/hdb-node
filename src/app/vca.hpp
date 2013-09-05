#ifndef VCA_HPP
#define VCA_HPP

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <string>

namespace app
{

using boost::shared_ptr;
using boost::thread;
using std::string;

class vca
{
public:
    vca ( string const& name ) : name_ ( name ), running_ ( false ) {}
    ~vca() {}
    virtual void init() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void restart() = 0;
    virtual bool running() { return running_; }

public:
    virtual string desc() const = 0;
    string const& name() { return name_; }
    void name ( string const& value ) { name_ = value; }
    string const& type() { return type_; }
    void type ( string const& value ) { type_ = value; }
    ptree const& parameters() const { return parameters_; }
    void parameters ( ptree const& value ) { parameters_ = value; }

protected:
    string name_;
    string type_;
    path working_dir_;
    ptree parameters_;
    bool running_;
};

}

#endif
