/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef IndoorLocationService_H
#define IndoorLocationService_H

#include <TProcessor.h>
#include "CoreServices_types.h"

namespace com { namespace kaisquare { namespace core { namespace thrift {

class IndoorLocationServiceIf {
 public:
  virtual ~IndoorLocationServiceIf() {}
  virtual void getMaps(std::vector<IndoorMapInfo> & _return, const std::string& deviceId) = 0;
  virtual void getTags(std::vector<TagInfo> & _return, const std::string& deviceId) = 0;
  virtual void getTagLocation(std::vector<IndoorLocationInfo> & _return, const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp) = 0;
};

class IndoorLocationServiceIfFactory {
 public:
  typedef IndoorLocationServiceIf Handler;

  virtual ~IndoorLocationServiceIfFactory() {}

  virtual IndoorLocationServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(IndoorLocationServiceIf* /* handler */) = 0;
};

class IndoorLocationServiceIfSingletonFactory : virtual public IndoorLocationServiceIfFactory {
 public:
  IndoorLocationServiceIfSingletonFactory(const boost::shared_ptr<IndoorLocationServiceIf>& iface) : iface_(iface) {}
  virtual ~IndoorLocationServiceIfSingletonFactory() {}

  virtual IndoorLocationServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(IndoorLocationServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<IndoorLocationServiceIf> iface_;
};

class IndoorLocationServiceNull : virtual public IndoorLocationServiceIf {
 public:
  virtual ~IndoorLocationServiceNull() {}
  void getMaps(std::vector<IndoorMapInfo> & /* _return */, const std::string& /* deviceId */) {
    return;
  }
  void getTags(std::vector<TagInfo> & /* _return */, const std::string& /* deviceId */) {
    return;
  }
  void getTagLocation(std::vector<IndoorLocationInfo> & /* _return */, const std::string& /* deviceId */, const std::string& /* tagId */, const std::string& /* startTimestamp */, const std::string& /* endTimestamp */) {
    return;
  }
};

typedef struct _IndoorLocationService_getMaps_args__isset {
  _IndoorLocationService_getMaps_args__isset() : deviceId(false) {}
  bool deviceId;
} _IndoorLocationService_getMaps_args__isset;

class IndoorLocationService_getMaps_args {
 public:

  IndoorLocationService_getMaps_args() : deviceId("") {
  }

  virtual ~IndoorLocationService_getMaps_args() throw() {}

  std::string deviceId;

  _IndoorLocationService_getMaps_args__isset __isset;

  void __set_deviceId(const std::string& val) {
    deviceId = val;
  }

  bool operator == (const IndoorLocationService_getMaps_args & rhs) const
  {
    if (!(deviceId == rhs.deviceId))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getMaps_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getMaps_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class IndoorLocationService_getMaps_pargs {
 public:


  virtual ~IndoorLocationService_getMaps_pargs() throw() {}

  const std::string* deviceId;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getMaps_result__isset {
  _IndoorLocationService_getMaps_result__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getMaps_result__isset;

class IndoorLocationService_getMaps_result {
 public:

  IndoorLocationService_getMaps_result() {
  }

  virtual ~IndoorLocationService_getMaps_result() throw() {}

  std::vector<IndoorMapInfo>  success;

  _IndoorLocationService_getMaps_result__isset __isset;

  void __set_success(const std::vector<IndoorMapInfo> & val) {
    success = val;
  }

  bool operator == (const IndoorLocationService_getMaps_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getMaps_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getMaps_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getMaps_presult__isset {
  _IndoorLocationService_getMaps_presult__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getMaps_presult__isset;

class IndoorLocationService_getMaps_presult {
 public:


  virtual ~IndoorLocationService_getMaps_presult() throw() {}

  std::vector<IndoorMapInfo> * success;

  _IndoorLocationService_getMaps_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _IndoorLocationService_getTags_args__isset {
  _IndoorLocationService_getTags_args__isset() : deviceId(false) {}
  bool deviceId;
} _IndoorLocationService_getTags_args__isset;

class IndoorLocationService_getTags_args {
 public:

  IndoorLocationService_getTags_args() : deviceId("") {
  }

  virtual ~IndoorLocationService_getTags_args() throw() {}

  std::string deviceId;

  _IndoorLocationService_getTags_args__isset __isset;

  void __set_deviceId(const std::string& val) {
    deviceId = val;
  }

  bool operator == (const IndoorLocationService_getTags_args & rhs) const
  {
    if (!(deviceId == rhs.deviceId))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getTags_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getTags_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class IndoorLocationService_getTags_pargs {
 public:


  virtual ~IndoorLocationService_getTags_pargs() throw() {}

  const std::string* deviceId;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getTags_result__isset {
  _IndoorLocationService_getTags_result__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getTags_result__isset;

class IndoorLocationService_getTags_result {
 public:

  IndoorLocationService_getTags_result() {
  }

  virtual ~IndoorLocationService_getTags_result() throw() {}

  std::vector<TagInfo>  success;

  _IndoorLocationService_getTags_result__isset __isset;

  void __set_success(const std::vector<TagInfo> & val) {
    success = val;
  }

  bool operator == (const IndoorLocationService_getTags_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getTags_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getTags_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getTags_presult__isset {
  _IndoorLocationService_getTags_presult__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getTags_presult__isset;

class IndoorLocationService_getTags_presult {
 public:


  virtual ~IndoorLocationService_getTags_presult() throw() {}

  std::vector<TagInfo> * success;

  _IndoorLocationService_getTags_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _IndoorLocationService_getTagLocation_args__isset {
  _IndoorLocationService_getTagLocation_args__isset() : deviceId(false), tagId(false), startTimestamp(false), endTimestamp(false) {}
  bool deviceId;
  bool tagId;
  bool startTimestamp;
  bool endTimestamp;
} _IndoorLocationService_getTagLocation_args__isset;

class IndoorLocationService_getTagLocation_args {
 public:

  IndoorLocationService_getTagLocation_args() : deviceId(""), tagId(""), startTimestamp(""), endTimestamp("") {
  }

  virtual ~IndoorLocationService_getTagLocation_args() throw() {}

  std::string deviceId;
  std::string tagId;
  std::string startTimestamp;
  std::string endTimestamp;

  _IndoorLocationService_getTagLocation_args__isset __isset;

  void __set_deviceId(const std::string& val) {
    deviceId = val;
  }

  void __set_tagId(const std::string& val) {
    tagId = val;
  }

  void __set_startTimestamp(const std::string& val) {
    startTimestamp = val;
  }

  void __set_endTimestamp(const std::string& val) {
    endTimestamp = val;
  }

  bool operator == (const IndoorLocationService_getTagLocation_args & rhs) const
  {
    if (!(deviceId == rhs.deviceId))
      return false;
    if (!(tagId == rhs.tagId))
      return false;
    if (!(startTimestamp == rhs.startTimestamp))
      return false;
    if (!(endTimestamp == rhs.endTimestamp))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getTagLocation_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getTagLocation_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class IndoorLocationService_getTagLocation_pargs {
 public:


  virtual ~IndoorLocationService_getTagLocation_pargs() throw() {}

  const std::string* deviceId;
  const std::string* tagId;
  const std::string* startTimestamp;
  const std::string* endTimestamp;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getTagLocation_result__isset {
  _IndoorLocationService_getTagLocation_result__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getTagLocation_result__isset;

class IndoorLocationService_getTagLocation_result {
 public:

  IndoorLocationService_getTagLocation_result() {
  }

  virtual ~IndoorLocationService_getTagLocation_result() throw() {}

  std::vector<IndoorLocationInfo>  success;

  _IndoorLocationService_getTagLocation_result__isset __isset;

  void __set_success(const std::vector<IndoorLocationInfo> & val) {
    success = val;
  }

  bool operator == (const IndoorLocationService_getTagLocation_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const IndoorLocationService_getTagLocation_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const IndoorLocationService_getTagLocation_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _IndoorLocationService_getTagLocation_presult__isset {
  _IndoorLocationService_getTagLocation_presult__isset() : success(false) {}
  bool success;
} _IndoorLocationService_getTagLocation_presult__isset;

class IndoorLocationService_getTagLocation_presult {
 public:


  virtual ~IndoorLocationService_getTagLocation_presult() throw() {}

  std::vector<IndoorLocationInfo> * success;

  _IndoorLocationService_getTagLocation_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class IndoorLocationServiceClient : virtual public IndoorLocationServiceIf {
 public:
  IndoorLocationServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  IndoorLocationServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getMaps(std::vector<IndoorMapInfo> & _return, const std::string& deviceId);
  void send_getMaps(const std::string& deviceId);
  void recv_getMaps(std::vector<IndoorMapInfo> & _return);
  void getTags(std::vector<TagInfo> & _return, const std::string& deviceId);
  void send_getTags(const std::string& deviceId);
  void recv_getTags(std::vector<TagInfo> & _return);
  void getTagLocation(std::vector<IndoorLocationInfo> & _return, const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp);
  void send_getTagLocation(const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp);
  void recv_getTagLocation(std::vector<IndoorLocationInfo> & _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class IndoorLocationServiceProcessor : public ::apache::thrift::TProcessor {
 protected:
  boost::shared_ptr<IndoorLocationServiceIf> iface_;
  virtual bool process_fn(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid, void* callContext);
 private:
  std::map<std::string, void (IndoorLocationServiceProcessor::*)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*, void*)> processMap_;
  void process_getMaps(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getTags(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getTagLocation(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  IndoorLocationServiceProcessor(boost::shared_ptr<IndoorLocationServiceIf> iface) :
    iface_(iface) {
    processMap_["getMaps"] = &IndoorLocationServiceProcessor::process_getMaps;
    processMap_["getTags"] = &IndoorLocationServiceProcessor::process_getTags;
    processMap_["getTagLocation"] = &IndoorLocationServiceProcessor::process_getTagLocation;
  }

  virtual bool process(boost::shared_ptr<apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr<apache::thrift::protocol::TProtocol> poprot, void* callContext);
  virtual ~IndoorLocationServiceProcessor() {}
};

class IndoorLocationServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  IndoorLocationServiceProcessorFactory(const ::boost::shared_ptr< IndoorLocationServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< IndoorLocationServiceIfFactory > handlerFactory_;
};

class IndoorLocationServiceMultiface : virtual public IndoorLocationServiceIf {
 public:
  IndoorLocationServiceMultiface(std::vector<boost::shared_ptr<IndoorLocationServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~IndoorLocationServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<IndoorLocationServiceIf> > ifaces_;
  IndoorLocationServiceMultiface() {}
  void add(boost::shared_ptr<IndoorLocationServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void getMaps(std::vector<IndoorMapInfo> & _return, const std::string& deviceId) {
    size_t sz = ifaces_.size();
    for (size_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getMaps(_return, deviceId);
        return;
      } else {
        ifaces_[i]->getMaps(_return, deviceId);
      }
    }
  }

  void getTags(std::vector<TagInfo> & _return, const std::string& deviceId) {
    size_t sz = ifaces_.size();
    for (size_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getTags(_return, deviceId);
        return;
      } else {
        ifaces_[i]->getTags(_return, deviceId);
      }
    }
  }

  void getTagLocation(std::vector<IndoorLocationInfo> & _return, const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp) {
    size_t sz = ifaces_.size();
    for (size_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getTagLocation(_return, deviceId, tagId, startTimestamp, endTimestamp);
        return;
      } else {
        ifaces_[i]->getTagLocation(_return, deviceId, tagId, startTimestamp, endTimestamp);
      }
    }
  }

};

}}}} // namespace

#endif
