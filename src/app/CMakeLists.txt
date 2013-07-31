include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
include_directories(${cURL_INCLUDE_DIR})
link_directories(${cURL_LIBRARY_DIR})
include_directories(${sqlite_INCLUDE_DIR})
link_directories(${sqlite_LIBRARY_DIR})
include_directories(${glog_INCLUDE_DIR})
link_directories(${glog_LIBRARY_DIR})
include_directories(${openssl_INCLUDE_DIR})
link_directories(${openssl_LIBRARY_DIR})
include_directories(${jsoncpp_INCLUDE_DIR})
link_directories(${jsoncpp_LIBRARY_DIR})
include_directories(${rocket_INCLUDE_DIRS})
link_directories(${rocket_LIBRARY_DIR})
include_directories(${qpcpp_INCLUDE_DIR})
include_directories(${qpport_INCLUDE_DIR})
link_directories(${qpport_LIBRARY_DIR})
include_directories(${pstream_INCLUDE_DIR})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(../common)

set(APP_SRCS main.cpp global.cpp bsp.cpp ../common/common.cpp
    controller.cpp uploader.cpp vca_manager.cpp)

add_definitions(-std=c++11)
add_definitions(-DQ_EVT_VIRTUAL)
add_definitions(-DQ_EVT_CTOR)
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS)

add_executable(app ${APP_SRCS})
target_link_libraries(app
    pthread GL rt
    ${Boost_LIBRARIES} ${glog_LIBRARIES} ${cURL_LIBRARIES}
    ${sqlite_LIBRARIES} ${jsoncpp_LIBRARIES} ${qpport_LIBRARIES})

add_custom_command(TARGET app POST_BUILD
    COMMAND bash ${CMAKE_SOURCE_DIR}/script/cpld.sh
        ${CMAKE_CURRENT_BINARY_DIR}/app
        ${CMAKE_INSTALL_PREFIX}/${PACKAGE_VERSION}/${OS_VERSION}/node/app)

install(TARGETS app
        DESTINATION ${PACKAGE_VERSION}/${OS_VERSION}/node/app)
install(FILES app.conf app.conf.local
        DESTINATION ${PACKAGE_VERSION}/${OS_VERSION}/node/app)
install(DIRECTORY snapshots
        DESTINATION ${PACKAGE_VERSION}/${OS_VERSION}/node/app)
install(DIRECTORY bin/${OS_VERSION}/vca
        DESTINATION ${PACKAGE_VERSION}/${OS_VERSION}/node/app)
install(PROGRAMS bin/${OS_VERSION}/vca/vca
        DESTINATION ${PACKAGE_VERSION}/${OS_VERSION}/node/app/vca)
