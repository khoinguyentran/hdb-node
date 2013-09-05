#ifndef REPORT_MANAGER_HPP
#define REPORT_MANAGER_HPP

#include "analysis_manager.hpp"
#include "report.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace app
{

using boost::property_tree::ptree;
using boost::shared_ptr;
using std::vector;

class report_name_duplicated : public std::runtime_error
{
public:
    report_name_duplicated ( string const & name )
        : runtime_error ( "report_name_duplicated" ),
        name_ ( name ) {}
    string name_;
};

class report_manager : boost::noncopyable
{
public:
    report_manager ( shared_ptr< analysis_manager > analysismgr )
        : analysismgr_ ( analysismgr ) {}
    ~report_manager() {}
    void load ( ptree const & );
    void start_all();
    void stop_all();

private:
    void add ( shared_ptr< report > );

private:
    vector< shared_ptr < report > > reports_;
    shared_ptr< analysis_manager > analysismgr_;

};

}

#endif
