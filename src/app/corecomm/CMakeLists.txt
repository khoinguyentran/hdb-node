include_directories(${thrift_INCLUDE_DIR})
link_directories(${thrift_LIBRARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DHAVE_NETINET_IN_H)
add_definitions(-DHAVE_INTTYPES_H)

add_library(corecomm
    CoreServices_constants.cpp
    CoreServices_types.cpp
    DataService.cpp
    DeviceControlService.cpp
    DeviceManagementService.cpp
    IndoorLocationService.cpp
    StreamControlService.cpp
    EventStructures_constants.cpp
    EventStructures_types.cpp
)

target_link_libraries(corecomm ${thrift_LIBRARIES} ${Boost_LIBRARIES})