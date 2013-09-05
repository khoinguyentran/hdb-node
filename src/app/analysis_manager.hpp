#ifndef ANALYSIS_MANAGER_HPP
#define ANALYSIS_MANAGER_HPP

#include "analysis.hpp"
#include "device_manager.hpp"

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

class analysis_name_duplicated : public std::runtime_error
{
public:
    analysis_name_duplicated ( string const & name )
        : runtime_error ( "analysis_name_duplicated" ),
        name_ ( name ) {}
    string name_;
};

class analysis_manager
{
public:
    analysis_manager ( shared_ptr< device_manager > devmgr ) { devmgr_ = devmgr; }
    ~analysis_manager() {}

    void load ( ptree const & );
    void start_all();
    void stop_all();
    shared_ptr< analysis > find_by_name ( string const & );
    vector< shared_ptr< analysis > > find_by_type ( string const & );

private:
    void add ( shared_ptr< analysis > );

private:
    vector< shared_ptr< analysis > > analyses_;
    shared_ptr< device_manager > devmgr_;
};

}

#endif
