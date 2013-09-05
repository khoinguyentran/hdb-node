#include "common.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <iostream>
#include <queue>
#include <sstream>

namespace common
{
using boost::make_shared;
using boost::shared_ptr;
using boost::posix_time::microsec_clock;
using boost::posix_time::second_clock;
using boost::posix_time::time_facet;
using boost::posix_time::time_input_facet;
using std::cout;
using std::endl;
using std::istringstream;
using std::stringstream;
using std::ostringstream;
using std::queue;

void print_ptree(ptree const& pt)
{
    ptree::const_iterator end = pt.end();

    for (ptree::const_iterator it = pt.begin(); it != end; ++it)
    {
        cout << it->first << ": " << it->second.get_value<string>() << endl;
        print_ptree(it->second);
    }
}

ptree merge_ptree(const boost::property_tree::ptree& rptFirst,
                  const boost::property_tree::ptree& rptSecond )
{
    //Take over first property tree
    boost::property_tree::ptree ptMerged = rptFirst;

    // Keep track of keys and values (subtrees) in second property tree
    queue<string> qKeys;
    queue<boost::property_tree::ptree> qValues;
    qValues.push( rptSecond );

    // Iterate over second property tree
    while( !qValues.empty() )
    {
        // Setup keys and corresponding values
        boost::property_tree::ptree ptree = qValues.front();
        qValues.pop();
        string keychain = "";
        if( !qKeys.empty() )
        {
            keychain = qKeys.front();
            qKeys.pop();
        }

        // Iterate over keys level-wise
        BOOST_FOREACH( const boost::property_tree::ptree::value_type& child, ptree )
        {
            // Leaf
            if( child.second.size() == 0 )
            {
                // No "." for first level entries
                string s;
                if( keychain != "" )
                    s = keychain + "." + child.first.data();
                else
                    s = child.first.data();

                // Put into combined property tree
                ptMerged.put( s, child.second.data() );
            }
            // Subtree
            else
            {
                // Put keys (identifiers of subtrees) and all of its parents (where present)
                // aside for later iteration. Keys on first level have no parents
                if( keychain != "" )
                    qKeys.push( keychain + "." + child.first.data() );
                else
                    qKeys.push( child.first.data() );

                // Put values (the subtrees) aside, too
                qValues.push( child.second );
            }
        }  // -- End of BOOST_FOREACH
    }  // --- End of while

    return ptMerged;
}

void load_config(shared_ptr< ptree > config,
                 path& global_path, path& local_path,
                 int argc, char* argv[])
{
    // Read global configs.
    auto global_config = make_shared< ptree >();
    read_ini(global_path.string(), *global_config);

    /* Read local config */
    auto local_config = make_shared< ptree >();
    read_ini(local_path.string(), *local_config);

    /* Overwrite global_config with local_config */
    *config = merge_ptree(*global_config, *local_config);
}

string get_utc_string(ptime const& the_time)
{
    auto facet = new time_facet("%Y-%m-%d %H:%M:%S");
    ostringstream oss;
    oss.imbue(std::locale(oss.getloc(), facet));
    oss << the_time;
    string str = oss.str();
    return str;
}

string get_simple_utc_string(ptime const& the_time)
{
    auto facet = new time_facet ("%Y-%m-%d %H-%M-%S");
    ostringstream oss;
    oss.imbue(std::locale(oss.getloc(), facet));
    oss << the_time;
    string str = oss.str();
    return str;
}

ptime parse_utc_string(string const& str)
{
    auto facet = new time_input_facet ("%Y-%m-%d %H:%M:%S");
    ptime pt;
    stringstream ss;
    ss.imbue(std::locale(ss.getloc(), facet));
    ss << str;
    ss >> pt;
    return pt;
}

ptime parse_simple_utc_string(string const& str)
{
    auto facet = new time_input_facet ("%Y-%m-%d %H-%M-%S");
    ptime pt;
    stringstream ss;
    ss.imbue(std::locale(ss.getloc(), facet));
    ss << str;
    ss >> pt;
    return pt;
}

string get_ddMMyyyyHHmmss_utc_string(ptime const& the_time)
{
    auto facet = new time_facet ("%d%m%Y%H%M%S");
    ostringstream oss;
    oss.imbue(std::locale(oss.getloc(), facet));
    oss << the_time;
    string str = oss.str();
    return str;
}

}
