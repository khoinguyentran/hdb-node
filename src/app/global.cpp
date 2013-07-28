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
    ifstream version_file("../VERSION");
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

void init_libraries()
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
static QP::QActive* default_uploader;
static QP::QActive* default_vca_manager;

static QP::QEvt const** default_controller_equeue;
static QP::QEvt const** default_uploader_equeue;
static QP::QEvt const** default_vca_manager_equeue;

static QP::QEvt* small_pool;

QP::QActive* get_default_controller() { return default_controller; }
QP::QActive* get_default_uploader() { return default_uploader; }
QP::QActive* get_default_vca_manager() { return default_vca_manager; }


void qp_init()
{
    /* Allocate event queues */
    size_t controller_equeue_size =
        global::config()->get("qp.controller_equeue_size", 50);
    size_t uploader_equeue_size =
        global::config()->get("qp.uploader_equeue_size", 50);
    size_t vca_manager_equeue_size =
        global::config()->get("qp.vca_manager_equeue_size", 50);

    default_controller_equeue = new QP::QEvt const*[controller_equeue_size];
    default_uploader_equeue = new QP::QEvt const*[uploader_equeue_size];
    default_vca_manager_equeue = new QP::QEvt const*[vca_manager_equeue_size];

    /* Initialize the qp framework */
    QP::QF::init();

    /* Initialize event pools */
    size_t small_pool_size = global::config()->get("qp.small_pool_size", 100);
    small_pool = new app::gevt[small_pool_size];
    QP::QF::poolInit(small_pool, small_pool_size, sizeof(app::gevt));

    /* Create components */
    default_controller = app::create_controller();
    default_uploader = app::create_uploader();    
    default_vca_manager = app::create_vca_manager();

    /* Start components */
    default_controller->start(
        3, default_controller_equeue, controller_equeue_size,
        (void *)0, 1024, (QP::QEvt*)0);
    default_uploader->start(
        2, default_uploader_equeue, uploader_equeue_size,
        (void *)0, 1024, (QP::QEvt*)0);
    default_vca_manager->start(
        1, default_vca_manager_equeue, vca_manager_equeue_size,
        (void *)0, 1024, (QP::QEvt*)0);
}

int run()
{
    return QP::QF::run();
}

sqlite3* sql;
void init_database()
{
    int error;
    
    LOG(INFO) << "sqlite3_libversion=" << sqlite3_libversion();
    LOG(INFO) << "sqlite3_threadsafe=" << sqlite3_threadsafe();
    
    error = sqlite3_open("db.sqlite3", &sql);
    if (error)
    {
        LOG(INFO) << "Could not initialize database.";
        throw "could not initialize database";
    }
    
    error = sqlite3_exec(sql,
                         "CREATE TABLE IF NOT EXISTS vca_events \
                         (id INTEGER PRIMARY KEY, \
                         timestamp DATETIME NOT NULL, \
                         description TEXT NOT NULL, \         
                         snapshot_path TEXT, \
                         uploaded INTEGER)",
                         0, 0, 0);
    if (error)
    {
        LOG(INFO) << "Could not create table events.";
        throw "could not create table events";
    }
}

sqlite3* get_database_handle()
{
    return sql;
}

}
