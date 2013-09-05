#ifndef SERVICE_MANAGER_HPP
#define SERVICE_MANAGER_HPP

#include "analysis_manager.hpp"
#include "vca.hpp"

#include <boost/property_tree/ptree.hpp>

#include <vector>

namespace app
{

using boost::property_tree::ptree;
using boost::shared_ptr;
using std::vector;

class service_manager
{
public:
    service_manager ( shared_ptr< analysis_manager > analysismgr ) : analysismgr_ ( analysismgr ) {}
    ~service_manager() {}

public:
    void load ( ptree const & );
    void start_all();
    void stop_all();
    void resume_all();
    vector< shared_ptr< vca > > find_by_type ( string const & );

public:
    void add ( shared_ptr< vca > );

private:
    vector< shared_ptr< vca > > vcas_;
    shared_ptr< analysis_manager > analysismgr_;

};


}

#endif
