#include "DeviceManagementService.h"

#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>

#include <protocol/TBinaryProtocol.h>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <vector>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace com::kaisquare::core::thrift;

using boost::shared_ptr;
using std::vector;

int main(int argc, char **argv) {
    std::cout << "Core Thrift Test" << std::endl;
    
    shared_ptr< TSocket > socket ( new TSocket ( "192.168.0.100", 10889 ) );
    shared_ptr< TTransport > transport ( new TFramedTransport ( socket ) );
    shared_ptr< TProtocol > protocol ( new TBinaryProtocol ( transport ) );
    DeviceManagementServiceClient dmsc ( protocol );
    
    try
    {
        std::cout << "Connecting to thrift server..." << std::endl;    
        transport->open();
        
        std::cout << "Retrieving device models..." << std::endl;    
        vector< DeviceModel > dev_models;
        
        dmsc.listModels ( dev_models );
        
        BOOST_FOREACH ( DeviceModel model, dev_models )
        {
            std::cout << model.name << std::endl;
        }
        
        transport->close();
    }
    catch (std::exception const& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
