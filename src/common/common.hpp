#ifndef COMMON_HPP
#define COMMON_HPP

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <string>

namespace common
{
using namespace boost::filesystem;
using boost::property_tree::ptree;
using boost::shared_ptr;
using boost::posix_time::ptime;
using std::string;

void print_ptree(ptree const &);
ptree merge_ptree(ptree const &, ptree const &);
void load_config(shared_ptr< ptree >, path&, path&, int, char* []);
string get_utc_string(ptime const &);
ptime parse_utc_string(string const &);
string get_simple_utc_string(ptime const &);
ptime parse_simple_utc_string(string const &);
string get_ddMMyyyyHHmmss_utc_string(ptime const &);
}

#endif
