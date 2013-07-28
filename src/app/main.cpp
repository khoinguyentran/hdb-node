#include "global.hpp"

#include <glog/logging.h>

#include <boost/filesystem.hpp>

#include <string>

int main(int argc, char* argv[])
{
    using namespace boost::filesystem;
    using std::string;

    // Configure logging facility.
    FLAGS_log_dir = ".";
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0]);

    path global("app.conf");
    path local("app.conf.local");
    global::load_config(global, local, argc, argv);

    global::init_libraries();
    global::init_database();

    global::qp_init();

    return global::run();
}
