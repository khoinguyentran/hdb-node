#include "global.hpp"
#include "common.hpp"
#include "fsm.hpp"
#include "version.hpp"

#include <glog/logging.h>
#include <curl/curl.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>

namespace global
{
using boost::make_shared;
using std::ifstream;

shared_ptr< ptree > configuration = make_shared< ptree >();

shared_ptr< ptree > config()
{
    return configuration;
}

void load_config(path& global_path, path& local_path,
                 int argc, char* argv[])
{
    LOG(INFO) << "Loading configuration...";
    common::load_config(configuration, global_path, local_path, argc, argv);

    configuration->add("info.os_version", OS_VERSION);
    
    configuration->add("info.module_name",
                       boost::filesystem::system_complete(argv[0]).filename());
    // Get current software version.
    ifstream version_file("VERSION");
    string version;
    if (version_file >> version)
    {
        configuration->add("info.package_version", version);
    }
    else
    {
        LOG(ERROR) << "Could not determine package version from VERSION file. Use constant in executable.";        
        configuration->add("info.package_version", PACKAGE_VERSION);
    }
    
    common::print_ptree(*configuration);    
}

void lib_init()
{
    CURLcode curl_error;
    curl_error = curl_global_init(CURL_GLOBAL_ALL);
    if (curl_error)
    {
        LOG(ERROR) << "Could not initialize curl.";
    }
}

// QP variables.
static QP::QActive* default_controller;
static QP::QActive* default_downloader;

static QP::QEvt const** default_controller_equeue;
static QP::QEvt const** default_downloader_equeue;

static QP::QEvt* small_pool;

QP::QActive* get_default_controller() {
    return default_controller;
}
QP::QActive* get_default_downloader() {
    return default_downloader;
}

void qp_init()
{
    /* Allocate event queues */
    size_t controller_equeue_size =
        global::config()->get("qp.controller_equeue_size", 50);
    size_t downloader_equeue_size =
        global::config()->get("qp.downloader_equeue_size", 50);

    default_controller_equeue = new QP::QEvt const*[controller_equeue_size];
    default_downloader_equeue = new QP::QEvt const*[downloader_equeue_size];

    /* Initialize the qp framework */
    QP::QF::init();

    /* Initialize event pools */
    size_t small_pool_size = global::config()->get("qp.small_pool_size", 100);
    small_pool = new controller::gevt[small_pool_size];
    QP::QF::poolInit(small_pool, small_pool_size, sizeof(controller::gevt));

    /* Create components */
    default_controller = controller::create_controller();
    default_downloader = controller::create_downloader();    

    /* Start components */
    default_controller->start(
        3, default_controller_equeue, controller_equeue_size,
        (void *)0, 1024, (QP::QEvt*)0);
    default_downloader->start(
        2, default_downloader_equeue, downloader_equeue_size,
        (void *)0, 1024, (QP::QEvt*)0);
}

int run()
{
    return QP::QF::run();
}

}
