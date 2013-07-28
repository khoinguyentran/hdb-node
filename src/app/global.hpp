#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <qp_port.h>
#include <sqlite3.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>

#include <string>

namespace global
{
using namespace boost::filesystem;
using boost::property_tree::ptree;
using boost::shared_ptr;
using std::string;

void init_libraries();

// Global configuration.
shared_ptr< ptree > config();
void load_config(path&, path&, int, char* []);

// QP functions.
void qp_init();
QP::QActive* get_default_controller();
QP::QActive* get_default_uploader();
QP::QActive* get_default_vca_manager();

int run();

// Database functions
void init_database();
sqlite3* get_database_handle();

}

#endif
