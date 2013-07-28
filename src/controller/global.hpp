#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <qp_port.h>

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

void lib_init();

// Global configuration.
shared_ptr< ptree > config();
void load_config(path&, path&, int, char* []);

// QP functions.
void qp_init();
QP::QActive* get_default_controller();
QP::QActive* get_default_downloader();

int run();
}

#endif
