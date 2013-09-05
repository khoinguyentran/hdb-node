#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "device.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace app
{

using boost::property_tree::ptree;
using boost::shared_ptr;
using std::string;
using std::vector;

class device_name_duplicated : public std::runtime_error
{
public:
    device_name_duplicated ( string const & name )
        : runtime_error ( "device_name_duplicated" ),
        name_ ( name ) {}
    string name_;
};

class device_manager
{
public:
    device_manager() {}
    ~device_manager() {}

public:
    void load ( ptree const & );
    void start_all();
    void stop_all();
    shared_ptr< device > find_by_name ( string const & );

private:
    void add ( shared_ptr< device > );

private:
    vector< shared_ptr< device > > devices_;
};

}

#endif
