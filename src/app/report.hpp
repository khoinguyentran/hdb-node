#ifndef REPORT_HPP
#define REPORT_HPP

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/utility.hpp>

#include <string>

namespace app
{

using namespace boost::filesystem;
using boost::property_tree::ptree;
using std::string;

class report : boost::noncopyable
{
public:
    report ( string const& name ) : name_ ( name ) {}
    ~report() {}
    virtual void start() = 0;
    virtual void stop() = 0;

public:
    virtual string desc() const = 0;
    virtual string const& name() const { return name_; }
    virtual void name ( string const& value ) { name_ = value; }
    virtual string const& type() const { return type_; }
    virtual void type ( string const& value ) { type_ = value; }
    virtual void parameters ( ptree const& value ) { parameters_ = value; }

protected:
    string name_;
    string type_;
    ptree parameters_;
    path working_dir_;
};
}

#endif
