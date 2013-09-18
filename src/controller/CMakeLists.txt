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
include_directories(${qpcpp_INCLUDE_DIR})
include_directories(${qpport_INCLUDE_DIR})
link_directories(${qpport_LIBRARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(../common)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in
    ${CMAKE_CURRENT_BINARY_DIR}/VERSION)

set(CONTROLLER_SRCS main.cpp
    ../common/common.cpp global.cpp bsp.cpp
    controller.cpp downloader.cpp)

add_definitions(-std=c++11)
add_definitions(-DQ_EVT_CTOR)
add_definitions(-DQ_EVT_VIRTUAL)
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS)

add_executable(controller ${CONTROLLER_SRCS})
target_link_libraries(controller
    pthread
    ${Boost_LIBRARIES} ${cURL_LIBRARIES}
    ${sqlite_LIBRARIES} ${glog_LIBRARIES} ${openssl_LIBRARIES}
    ${jsoncpp_LIBRARIES} ${qpport_LIBRARIES}
    rt)

set(PACKAGE_INSTALL_DIR
    ${CMAKE_INSTALL_PREFIX}/${OS_VERSION}/controller/${PACKAGE_VERSION})
set(PACKAGE_SCRIPT
    ${CMAKE_SOURCE_DIR}/script/prepare-package.sh)
set(PACKAGE_CODE "execute_process(COMMAND "
    "${PACKAGE_SCRIPT} "
    "controller "
    "${PACKAGE_VERSION} "
    "${OS_VERSION} "
    "${PACKAGE_INSTALL_DIR} ) ")

add_custom_command(TARGET controller POST_BUILD
    COMMAND bash ${CMAKE_SOURCE_DIR}/script/cpld.sh
        ${CMAKE_CURRENT_BINARY_DIR}/controller
        ${PACKAGE_INSTALL_DIR})

install(TARGETS controller
        DESTINATION ${OS_VERSION}/controller/${PACKAGE_VERSION})
install(DIRECTORY assets data
        DESTINATION ${OS_VERSION}/controller/${PACKAGE_VERSION})
install(PROGRAMS service/run
        DESTINATION ${OS_VERSION}/controller/${PACKAGE_VERSION}/service)
install(FILES controller.conf controller.conf.local
        DESTINATION ${OS_VERSION}/controller/${PACKAGE_VERSION})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
        DESTINATION ${OS_VERSION}/controller/${PACKAGE_VERSION})
install(CODE "${PACKAGE_CODE}")
