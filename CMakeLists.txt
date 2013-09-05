cmake_minimum_required(VERSION 2.8)
project(node)

# System architecture
message("System: ${CMAKE_SYSTEM_NAME}")
message("CPU: ${CMAKE_SYSTEM_PROCESSOR}")

# Version
if("$ENV{PACKAGE_VERSION}" STREQUAL "")
    set(PACKAGE_VERSION 1.0)
else()
    set(PACKAGE_VERSION $ENV{PACKAGE_VERSION})
endif()
message("Building for version ${PACKAGE_VERSION}...")

set(OS_VERSION "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Build options

# Library repository
if("$ENV{LIBREPO}" STREQUAL "")
    set(LIBREPO ~/Libraries)
else()
    set(LIBREPO $ENV{LIBREPO})
endif()

# qp state machine framework
set(qpcpp_ROOT_DIR ${LIBREPO}/qpcpp-4.5.04)
set(qpport_INCLUDE_DIR ${qpcpp_ROOT_DIR}/ports/posix/gnu)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")        
    set(qpport_LIBRARY_DIR ${qpcpp_ROOT_DIR}/ports/posix/gnu/dbg)
else()
    set(qpport_LIBRARY_DIR ${qpcpp_ROOT_DIR}/ports/posix/gnu/rel)
endif()
set(qpport_LIBRARIES qp)
set(qpcpp_INCLUDE_DIR ${qpcpp_ROOT_DIR}/include)

# Boost
set(BOOST_ROOT ${LIBREPO}/boost_1_53_0)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.53.0 COMPONENTS date_time regex system thread filesystem REQUIRED)

# cURL
set(cURL_INCLUDE_DIR ${LIBREPO}/curl-7.31.0/include)
set(cURL_LIBRARY_DIR ${LIBREPO}/curl-7.31.0/lib/.libs)
set(cURL_LIBRARIES curl)

# sqlite
set(sqlite_INCLUDE_DIR ${LIBREPO}/sqlite-autoconf-3071700)
set(sqlite_LIBRARY_DIR ${LIBREPO}/sqlite-autoconf-3071700/.libs)
set(sqlite_LIBRARIES sqlite3)

# glog
set(glog_INCLUDE_DIR ${LIBREPO}/glog-0.3.3/src)
set(glog_LIBRARY_DIR ${LIBREPO}/glog-0.3.3/.libs)
set(glog_LIBRARIES glog)

# cpld utility
set(CPLD_COMMAND cpld.sh)

# openssl
set(openssl_INCLUDE_DIR ${LIBREPO}/openssl-fips-ecp-2.0.4/include)
set(openssl_LIBRARY_DIR ${LIBREPO}/openssl-fips-ecp-2.0.4)
set(openssl_LIBRARIES crypto)

# jsoncpp
set(jsoncpp_INCLUDE_DIR ${LIBREPO}/jsoncpp-src-0.5.0/include)
set(jsoncpp_LIBRARY_DIR ${LIBREPO}/jsoncpp-src-0.5.0/libs/linux-gcc-4.8)
set(jsoncpp_LIBRARIES json_linux-gcc-4.8_libmt)

# pstream
set(pstream_INCLUDE_DIR ${LIBREPO}/pstreams-0.8.0)

# thrift
set(thrift_INCLUDE_DIR ${LIBREPO}/thrift-0.8.0/lib/cpp/src)
set(thrift_LIBRARY_DIR ${LIBREPO}/thrift-0.8.0/lib/cpp/.libs)
set(thrift_LIBRARIES thrift)

add_subdirectory(src)
