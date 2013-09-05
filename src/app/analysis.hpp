#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

#include "squeue.hpp"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace app
{

using namespace boost::filesystem;
using boost::property_tree::ptree;
using boost::shared_ptr;
using std::string;
using std::vector;

class analysis
{
public:
    analysis ( string const& name ) : name_ ( name ) {}
    ~analysis() {}

public:
    virtual string desc() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    string const& name() const { return name_; }
    void name ( string const& value ) { name_ = value; }
    string const& type() const { return type_; }
    void type ( string const& value ) { type_ = value; }
    ptree const& parameters() const { return parameters_; }
    void parameters ( ptree const& value ) { parameters_ = value; }

protected:
    string name_;
    string type_;
    path working_dir_;
    ptree parameters_;
};

}

#endif
