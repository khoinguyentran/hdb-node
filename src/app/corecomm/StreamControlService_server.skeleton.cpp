// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "StreamControlService.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::com::kaisquare::core::thrift;

class StreamControlServiceHandler : virtual public StreamControlServiceIf {
 public:
  StreamControlServiceHandler() {
    // Your initialization goes here
  }

  void beginStreamSession(std::vector<std::string> & _return, const std::string& sessionId, const int64_t ttl, const std::string& type, const std::vector<std::string> & allowedClientIpAddresses, const std::string& deviceId, const std::string& channelId, const std::string& startTimestamp, const std::string& endTimeStamp) {
    // Your implementation goes here
    printf("beginStreamSession\n");
  }

  bool keepStreamSessionAlive(const std::string& sessionId, const int64_t ttl, const std::vector<std::string> & allowedClientIpAddresses) {
    // Your implementation goes here
    printf("keepStreamSessionAlive\n");
  }

  bool endStreamSession(const std::string& sessionId) {
    // Your implementation goes here
    printf("endStreamSession\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<StreamControlServiceHandler> handler(new StreamControlServiceHandler());
  shared_ptr<TProcessor> processor(new StreamControlServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

