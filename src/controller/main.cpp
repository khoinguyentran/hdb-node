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

    path global("controller.conf");
    path local("controller.conf.local");
    global::load_config(global, local, argc, argv);

    global::lib_init();

    global::qp_init();

    return global::run();
}
