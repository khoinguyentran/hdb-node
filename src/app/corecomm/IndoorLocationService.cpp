/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "IndoorLocationService.h"

namespace com { namespace kaisquare { namespace core { namespace thrift {

uint32_t IndoorLocationService_getMaps_args::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->deviceId);
          this->__isset.deviceId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getMaps_args::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getMaps_args");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->deviceId);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getMaps_pargs::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getMaps_pargs");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString((*(this->deviceId)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getMaps_result::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->success.clear();
            uint32_t _size98;
            ::apache::thrift::protocol::TType _etype101;
            iprot->readListBegin(_etype101, _size98);
            this->success.resize(_size98);
            uint32_t _i102;
            for (_i102 = 0; _i102 < _size98; ++_i102)
            {
              xfer += this->success[_i102].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getMaps_result::write(::apache::thrift::protocol::TProtocol* oprot) const {

  uint32_t xfer = 0;

  xfer += oprot->writeStructBegin("IndoorLocationService_getMaps_result");

  if (this->__isset.success) {
    xfer += oprot->writeFieldBegin("success", ::apache::thrift::protocol::T_LIST, 0);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->success.size()));
      std::vector<IndoorMapInfo> ::const_iterator _iter103;
      for (_iter103 = this->success.begin(); _iter103 != this->success.end(); ++_iter103)
      {
        xfer += (*_iter103).write(oprot);
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getMaps_presult::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            (*(this->success)).clear();
            uint32_t _size104;
            ::apache::thrift::protocol::TType _etype107;
            iprot->readListBegin(_etype107, _size104);
            (*(this->success)).resize(_size104);
            uint32_t _i108;
            for (_i108 = 0; _i108 < _size104; ++_i108)
            {
              xfer += (*(this->success))[_i108].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTags_args::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->deviceId);
          this->__isset.deviceId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTags_args::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getTags_args");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->deviceId);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTags_pargs::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getTags_pargs");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString((*(this->deviceId)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTags_result::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->success.clear();
            uint32_t _size109;
            ::apache::thrift::protocol::TType _etype112;
            iprot->readListBegin(_etype112, _size109);
            this->success.resize(_size109);
            uint32_t _i113;
            for (_i113 = 0; _i113 < _size109; ++_i113)
            {
              xfer += this->success[_i113].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTags_result::write(::apache::thrift::protocol::TProtocol* oprot) const {

  uint32_t xfer = 0;

  xfer += oprot->writeStructBegin("IndoorLocationService_getTags_result");

  if (this->__isset.success) {
    xfer += oprot->writeFieldBegin("success", ::apache::thrift::protocol::T_LIST, 0);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->success.size()));
      std::vector<TagInfo> ::const_iterator _iter114;
      for (_iter114 = this->success.begin(); _iter114 != this->success.end(); ++_iter114)
      {
        xfer += (*_iter114).write(oprot);
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTags_presult::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            (*(this->success)).clear();
            uint32_t _size115;
            ::apache::thrift::protocol::TType _etype118;
            iprot->readListBegin(_etype118, _size115);
            (*(this->success)).resize(_size115);
            uint32_t _i119;
            for (_i119 = 0; _i119 < _size115; ++_i119)
            {
              xfer += (*(this->success))[_i119].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_args::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->deviceId);
          this->__isset.deviceId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->tagId);
          this->__isset.tagId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->startTimestamp);
          this->__isset.startTimestamp = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->endTimestamp);
          this->__isset.endTimestamp = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_args::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getTagLocation_args");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->deviceId);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("tagId", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->tagId);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("startTimestamp", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->startTimestamp);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("endTimestamp", ::apache::thrift::protocol::T_STRING, 4);
  xfer += oprot->writeString(this->endTimestamp);
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_pargs::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("IndoorLocationService_getTagLocation_pargs");
  xfer += oprot->writeFieldBegin("deviceId", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString((*(this->deviceId)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("tagId", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString((*(this->tagId)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("startTimestamp", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString((*(this->startTimestamp)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldBegin("endTimestamp", ::apache::thrift::protocol::T_STRING, 4);
  xfer += oprot->writeString((*(this->endTimestamp)));
  xfer += oprot->writeFieldEnd();
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_result::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->success.clear();
            uint32_t _size120;
            ::apache::thrift::protocol::TType _etype123;
            iprot->readListBegin(_etype123, _size120);
            this->success.resize(_size120);
            uint32_t _i124;
            for (_i124 = 0; _i124 < _size120; ++_i124)
            {
              xfer += this->success[_i124].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_result::write(::apache::thrift::protocol::TProtocol* oprot) const {

  uint32_t xfer = 0;

  xfer += oprot->writeStructBegin("IndoorLocationService_getTagLocation_result");

  if (this->__isset.success) {
    xfer += oprot->writeFieldBegin("success", ::apache::thrift::protocol::T_LIST, 0);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->success.size()));
      std::vector<IndoorLocationInfo> ::const_iterator _iter125;
      for (_iter125 = this->success.begin(); _iter125 != this->success.end(); ++_iter125)
      {
        xfer += (*_iter125).write(oprot);
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

uint32_t IndoorLocationService_getTagLocation_presult::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            (*(this->success)).clear();
            uint32_t _size126;
            ::apache::thrift::protocol::TType _etype129;
            iprot->readListBegin(_etype129, _size126);
            (*(this->success)).resize(_size126);
            uint32_t _i130;
            for (_i130 = 0; _i130 < _size126; ++_i130)
            {
              xfer += (*(this->success))[_i130].read(iprot);
            }
            iprot->readListEnd();
          }
          this->__isset.success = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

void IndoorLocationServiceClient::getMaps(std::vector<IndoorMapInfo> & _return, const std::string& deviceId)
{
  send_getMaps(deviceId);
  recv_getMaps(_return);
}

void IndoorLocationServiceClient::send_getMaps(const std::string& deviceId)
{
  int32_t cseqid = 0;
  oprot_->writeMessageBegin("getMaps", ::apache::thrift::protocol::T_CALL, cseqid);

  IndoorLocationService_getMaps_pargs args;
  args.deviceId = &deviceId;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();
}

void IndoorLocationServiceClient::recv_getMaps(std::vector<IndoorMapInfo> & _return)
{

  int32_t rseqid = 0;
  std::string fname;
  ::apache::thrift::protocol::TMessageType mtype;

  iprot_->readMessageBegin(fname, mtype, rseqid);
  if (mtype == ::apache::thrift::protocol::T_EXCEPTION) {
    ::apache::thrift::TApplicationException x;
    x.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
    throw x;
  }
  if (mtype != ::apache::thrift::protocol::T_REPLY) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  if (fname.compare("getMaps") != 0) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  IndoorLocationService_getMaps_presult result;
  result.success = &_return;
  result.read(iprot_);
  iprot_->readMessageEnd();
  iprot_->getTransport()->readEnd();

  if (result.__isset.success) {
    // _return pointer has now been filled
    return;
  }
  throw ::apache::thrift::TApplicationException(::apache::thrift::TApplicationException::MISSING_RESULT, "getMaps failed: unknown result");
}

void IndoorLocationServiceClient::getTags(std::vector<TagInfo> & _return, const std::string& deviceId)
{
  send_getTags(deviceId);
  recv_getTags(_return);
}

void IndoorLocationServiceClient::send_getTags(const std::string& deviceId)
{
  int32_t cseqid = 0;
  oprot_->writeMessageBegin("getTags", ::apache::thrift::protocol::T_CALL, cseqid);

  IndoorLocationService_getTags_pargs args;
  args.deviceId = &deviceId;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();
}

void IndoorLocationServiceClient::recv_getTags(std::vector<TagInfo> & _return)
{

  int32_t rseqid = 0;
  std::string fname;
  ::apache::thrift::protocol::TMessageType mtype;

  iprot_->readMessageBegin(fname, mtype, rseqid);
  if (mtype == ::apache::thrift::protocol::T_EXCEPTION) {
    ::apache::thrift::TApplicationException x;
    x.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
    throw x;
  }
  if (mtype != ::apache::thrift::protocol::T_REPLY) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  if (fname.compare("getTags") != 0) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  IndoorLocationService_getTags_presult result;
  result.success = &_return;
  result.read(iprot_);
  iprot_->readMessageEnd();
  iprot_->getTransport()->readEnd();

  if (result.__isset.success) {
    // _return pointer has now been filled
    return;
  }
  throw ::apache::thrift::TApplicationException(::apache::thrift::TApplicationException::MISSING_RESULT, "getTags failed: unknown result");
}

void IndoorLocationServiceClient::getTagLocation(std::vector<IndoorLocationInfo> & _return, const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp)
{
  send_getTagLocation(deviceId, tagId, startTimestamp, endTimestamp);
  recv_getTagLocation(_return);
}

void IndoorLocationServiceClient::send_getTagLocation(const std::string& deviceId, const std::string& tagId, const std::string& startTimestamp, const std::string& endTimestamp)
{
  int32_t cseqid = 0;
  oprot_->writeMessageBegin("getTagLocation", ::apache::thrift::protocol::T_CALL, cseqid);

  IndoorLocationService_getTagLocation_pargs args;
  args.deviceId = &deviceId;
  args.tagId = &tagId;
  args.startTimestamp = &startTimestamp;
  args.endTimestamp = &endTimestamp;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();
}

void IndoorLocationServiceClient::recv_getTagLocation(std::vector<IndoorLocationInfo> & _return)
{

  int32_t rseqid = 0;
  std::string fname;
  ::apache::thrift::protocol::TMessageType mtype;

  iprot_->readMessageBegin(fname, mtype, rseqid);
  if (mtype == ::apache::thrift::protocol::T_EXCEPTION) {
    ::apache::thrift::TApplicationException x;
    x.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
    throw x;
  }
  if (mtype != ::apache::thrift::protocol::T_REPLY) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  if (fname.compare("getTagLocation") != 0) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  IndoorLocationService_getTagLocation_presult result;
  result.success = &_return;
  result.read(iprot_);
  iprot_->readMessageEnd();
  iprot_->getTransport()->readEnd();

  if (result.__isset.success) {
    // _return pointer has now been filled
    return;
  }
  throw ::apache::thrift::TApplicationException(::apache::thrift::TApplicationException::MISSING_RESULT, "getTagLocation failed: unknown result");
}

bool IndoorLocationServiceProcessor::process(boost::shared_ptr<apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr<apache::thrift::protocol::TProtocol> poprot, void* callContext) {

  ::apache::thrift::protocol::TProtocol* iprot = piprot.get();
  ::apache::thrift::protocol::TProtocol* oprot = poprot.get();
  std::string fname;
  ::apache::thrift::protocol::TMessageType mtype;
  int32_t seqid;

  iprot->readMessageBegin(fname, mtype, seqid);

  if (mtype != ::apache::thrift::protocol::T_CALL && mtype != ::apache::thrift::protocol::T_ONEWAY) {
    iprot->skip(::apache::thrift::protocol::T_STRUCT);
    iprot->readMessageEnd();
    iprot->getTransport()->readEnd();
    ::apache::thrift::TApplicationException x(::apache::thrift::TApplicationException::INVALID_MESSAGE_TYPE);
    oprot->writeMessageBegin(fname, ::apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return true;
  }

  return process_fn(iprot, oprot, fname, seqid, callContext);
}

bool IndoorLocationServiceProcessor::process_fn(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid, void* callContext) {
  std::map<std::string, void (IndoorLocationServiceProcessor::*)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*, void*)>::iterator pfn;
  pfn = processMap_.find(fname);
  if (pfn == processMap_.end()) {
    iprot->skip(apache::thrift::protocol::T_STRUCT);
    iprot->readMessageEnd();
    iprot->getTransport()->readEnd();
    ::apache::thrift::TApplicationException x(::apache::thrift::TApplicationException::UNKNOWN_METHOD, "Invalid method name: '"+fname+"'");
    oprot->writeMessageBegin(fname, ::apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return true;
  }
  (this->*(pfn->second))(seqid, iprot, oprot, callContext);
  return true;
}

void IndoorLocationServiceProcessor::process_getMaps(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
  void* ctx = NULL;
  if (this->eventHandler_.get() != NULL) {
    ctx = this->eventHandler_->getContext("IndoorLocationService.getMaps", callContext);
  }
  apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "IndoorLocationService.getMaps");

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preRead(ctx, "IndoorLocationService.getMaps");
  }

  IndoorLocationService_getMaps_args args;
  args.read(iprot);
  iprot->readMessageEnd();
  uint32_t bytes = iprot->getTransport()->readEnd();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postRead(ctx, "IndoorLocationService.getMaps", bytes);
  }

  IndoorLocationService_getMaps_result result;
  try {
    iface_->getMaps(result.success, args.deviceId);
    result.__isset.success = true;
  } catch (const std::exception& e) {
    if (this->eventHandler_.get() != NULL) {
      this->eventHandler_->handlerError(ctx, "IndoorLocationService.getMaps");
    }

    apache::thrift::TApplicationException x(e.what());
    oprot->writeMessageBegin("getMaps", apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return;
  }

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preWrite(ctx, "IndoorLocationService.getMaps");
  }

  oprot->writeMessageBegin("getMaps", apache::thrift::protocol::T_REPLY, seqid);
  result.write(oprot);
  oprot->writeMessageEnd();
  bytes = oprot->getTransport()->writeEnd();
  oprot->getTransport()->flush();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postWrite(ctx, "IndoorLocationService.getMaps", bytes);
  }
}

void IndoorLocationServiceProcessor::process_getTags(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
  void* ctx = NULL;
  if (this->eventHandler_.get() != NULL) {
    ctx = this->eventHandler_->getContext("IndoorLocationService.getTags", callContext);
  }
  apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "IndoorLocationService.getTags");

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preRead(ctx, "IndoorLocationService.getTags");
  }

  IndoorLocationService_getTags_args args;
  args.read(iprot);
  iprot->readMessageEnd();
  uint32_t bytes = iprot->getTransport()->readEnd();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postRead(ctx, "IndoorLocationService.getTags", bytes);
  }

  IndoorLocationService_getTags_result result;
  try {
    iface_->getTags(result.success, args.deviceId);
    result.__isset.success = true;
  } catch (const std::exception& e) {
    if (this->eventHandler_.get() != NULL) {
      this->eventHandler_->handlerError(ctx, "IndoorLocationService.getTags");
    }

    apache::thrift::TApplicationException x(e.what());
    oprot->writeMessageBegin("getTags", apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return;
  }

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preWrite(ctx, "IndoorLocationService.getTags");
  }

  oprot->writeMessageBegin("getTags", apache::thrift::protocol::T_REPLY, seqid);
  result.write(oprot);
  oprot->writeMessageEnd();
  bytes = oprot->getTransport()->writeEnd();
  oprot->getTransport()->flush();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postWrite(ctx, "IndoorLocationService.getTags", bytes);
  }
}

void IndoorLocationServiceProcessor::process_getTagLocation(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
  void* ctx = NULL;
  if (this->eventHandler_.get() != NULL) {
    ctx = this->eventHandler_->getContext("IndoorLocationService.getTagLocation", callContext);
  }
  apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "IndoorLocationService.getTagLocation");

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preRead(ctx, "IndoorLocationService.getTagLocation");
  }

  IndoorLocationService_getTagLocation_args args;
  args.read(iprot);
  iprot->readMessageEnd();
  uint32_t bytes = iprot->getTransport()->readEnd();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postRead(ctx, "IndoorLocationService.getTagLocation", bytes);
  }

  IndoorLocationService_getTagLocation_result result;
  try {
    iface_->getTagLocation(result.success, args.deviceId, args.tagId, args.startTimestamp, args.endTimestamp);
    result.__isset.success = true;
  } catch (const std::exception& e) {
    if (this->eventHandler_.get() != NULL) {
      this->eventHandler_->handlerError(ctx, "IndoorLocationService.getTagLocation");
    }

    apache::thrift::TApplicationException x(e.what());
    oprot->writeMessageBegin("getTagLocation", apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return;
  }

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preWrite(ctx, "IndoorLocationService.getTagLocation");
  }

  oprot->writeMessageBegin("getTagLocation", apache::thrift::protocol::T_REPLY, seqid);
  result.write(oprot);
  oprot->writeMessageEnd();
  bytes = oprot->getTransport()->writeEnd();
  oprot->getTransport()->flush();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postWrite(ctx, "IndoorLocationService.getTagLocation", bytes);
  }
}

::boost::shared_ptr< ::apache::thrift::TProcessor > IndoorLocationServiceProcessorFactory::getProcessor(const ::apache::thrift::TConnectionInfo& connInfo) {
  ::apache::thrift::ReleaseHandler< IndoorLocationServiceIfFactory > cleanup(handlerFactory_);
  ::boost::shared_ptr< IndoorLocationServiceIf > handler(handlerFactory_->getHandler(connInfo), cleanup);
  ::boost::shared_ptr< ::apache::thrift::TProcessor > processor(new IndoorLocationServiceProcessor(handler));
  return processor;
}
}}}} // namespace
