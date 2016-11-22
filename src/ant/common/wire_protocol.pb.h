// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ant/common/wire_protocol.proto

#ifndef PROTOBUF_ant_2fcommon_2fwire_5fprotocol_2eproto__INCLUDED
#define PROTOBUF_ant_2fcommon_2fwire_5fprotocol_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
#include "ant/common/common.pb.h"
// @@protoc_insertion_point(includes)

namespace ant {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
void protobuf_AssignDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
void protobuf_ShutdownFile_ant_2fcommon_2fwire_5fprotocol_2eproto();

class AppStatusPB;
class NodeInstancePB;
class ServerRegistrationPB;
class ServerEntryPB;

enum AppStatusPB_ErrorCode {
  AppStatusPB_ErrorCode_UNKNOWN_ERROR = 999,
  AppStatusPB_ErrorCode_OK = 0,
  AppStatusPB_ErrorCode_NOT_FOUND = 1,
  AppStatusPB_ErrorCode_CORRUPTION = 2,
  AppStatusPB_ErrorCode_NOT_SUPPORTED = 3,
  AppStatusPB_ErrorCode_INVALID_ARGUMENT = 4,
  AppStatusPB_ErrorCode_IO_ERROR = 5,
  AppStatusPB_ErrorCode_ALREADY_PRESENT = 6,
  AppStatusPB_ErrorCode_RUNTIME_ERROR = 7,
  AppStatusPB_ErrorCode_NETWORK_ERROR = 8,
  AppStatusPB_ErrorCode_ILLEGAL_STATE = 9,
  AppStatusPB_ErrorCode_NOT_AUTHORIZED = 10,
  AppStatusPB_ErrorCode_ABORTED = 11,
  AppStatusPB_ErrorCode_REMOTE_ERROR = 12,
  AppStatusPB_ErrorCode_SERVICE_UNAVAILABLE = 13,
  AppStatusPB_ErrorCode_TIMED_OUT = 14,
  AppStatusPB_ErrorCode_UNINITIALIZED = 15,
  AppStatusPB_ErrorCode_CONFIGURATION_ERROR = 16,
  AppStatusPB_ErrorCode_INCOMPLETE = 17,
  AppStatusPB_ErrorCode_END_OF_FILE = 18
};
bool AppStatusPB_ErrorCode_IsValid(int value);
const AppStatusPB_ErrorCode AppStatusPB_ErrorCode_ErrorCode_MIN = AppStatusPB_ErrorCode_OK;
const AppStatusPB_ErrorCode AppStatusPB_ErrorCode_ErrorCode_MAX = AppStatusPB_ErrorCode_UNKNOWN_ERROR;
const int AppStatusPB_ErrorCode_ErrorCode_ARRAYSIZE = AppStatusPB_ErrorCode_ErrorCode_MAX + 1;

const ::google::protobuf::EnumDescriptor* AppStatusPB_ErrorCode_descriptor();
inline const ::std::string& AppStatusPB_ErrorCode_Name(AppStatusPB_ErrorCode value) {
  return ::google::protobuf::internal::NameOfEnum(
    AppStatusPB_ErrorCode_descriptor(), value);
}
inline bool AppStatusPB_ErrorCode_Parse(
    const ::std::string& name, AppStatusPB_ErrorCode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<AppStatusPB_ErrorCode>(
    AppStatusPB_ErrorCode_descriptor(), name, value);
}
// ===================================================================

class AppStatusPB : public ::google::protobuf::Message {
 public:
  AppStatusPB();
  virtual ~AppStatusPB();

  AppStatusPB(const AppStatusPB& from);

  inline AppStatusPB& operator=(const AppStatusPB& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const AppStatusPB& default_instance();

  void Swap(AppStatusPB* other);

  // implements Message ----------------------------------------------

  AppStatusPB* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const AppStatusPB& from);
  void MergeFrom(const AppStatusPB& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef AppStatusPB_ErrorCode ErrorCode;
  static const ErrorCode UNKNOWN_ERROR = AppStatusPB_ErrorCode_UNKNOWN_ERROR;
  static const ErrorCode OK = AppStatusPB_ErrorCode_OK;
  static const ErrorCode NOT_FOUND = AppStatusPB_ErrorCode_NOT_FOUND;
  static const ErrorCode CORRUPTION = AppStatusPB_ErrorCode_CORRUPTION;
  static const ErrorCode NOT_SUPPORTED = AppStatusPB_ErrorCode_NOT_SUPPORTED;
  static const ErrorCode INVALID_ARGUMENT = AppStatusPB_ErrorCode_INVALID_ARGUMENT;
  static const ErrorCode IO_ERROR = AppStatusPB_ErrorCode_IO_ERROR;
  static const ErrorCode ALREADY_PRESENT = AppStatusPB_ErrorCode_ALREADY_PRESENT;
  static const ErrorCode RUNTIME_ERROR = AppStatusPB_ErrorCode_RUNTIME_ERROR;
  static const ErrorCode NETWORK_ERROR = AppStatusPB_ErrorCode_NETWORK_ERROR;
  static const ErrorCode ILLEGAL_STATE = AppStatusPB_ErrorCode_ILLEGAL_STATE;
  static const ErrorCode NOT_AUTHORIZED = AppStatusPB_ErrorCode_NOT_AUTHORIZED;
  static const ErrorCode ABORTED = AppStatusPB_ErrorCode_ABORTED;
  static const ErrorCode REMOTE_ERROR = AppStatusPB_ErrorCode_REMOTE_ERROR;
  static const ErrorCode SERVICE_UNAVAILABLE = AppStatusPB_ErrorCode_SERVICE_UNAVAILABLE;
  static const ErrorCode TIMED_OUT = AppStatusPB_ErrorCode_TIMED_OUT;
  static const ErrorCode UNINITIALIZED = AppStatusPB_ErrorCode_UNINITIALIZED;
  static const ErrorCode CONFIGURATION_ERROR = AppStatusPB_ErrorCode_CONFIGURATION_ERROR;
  static const ErrorCode INCOMPLETE = AppStatusPB_ErrorCode_INCOMPLETE;
  static const ErrorCode END_OF_FILE = AppStatusPB_ErrorCode_END_OF_FILE;
  static inline bool ErrorCode_IsValid(int value) {
    return AppStatusPB_ErrorCode_IsValid(value);
  }
  static const ErrorCode ErrorCode_MIN =
    AppStatusPB_ErrorCode_ErrorCode_MIN;
  static const ErrorCode ErrorCode_MAX =
    AppStatusPB_ErrorCode_ErrorCode_MAX;
  static const int ErrorCode_ARRAYSIZE =
    AppStatusPB_ErrorCode_ErrorCode_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  ErrorCode_descriptor() {
    return AppStatusPB_ErrorCode_descriptor();
  }
  static inline const ::std::string& ErrorCode_Name(ErrorCode value) {
    return AppStatusPB_ErrorCode_Name(value);
  }
  static inline bool ErrorCode_Parse(const ::std::string& name,
      ErrorCode* value) {
    return AppStatusPB_ErrorCode_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required .ant.AppStatusPB.ErrorCode code = 1;
  inline bool has_code() const;
  inline void clear_code();
  static const int kCodeFieldNumber = 1;
  inline ::ant::AppStatusPB_ErrorCode code() const;
  inline void set_code(::ant::AppStatusPB_ErrorCode value);

  // optional string message = 2;
  inline bool has_message() const;
  inline void clear_message();
  static const int kMessageFieldNumber = 2;
  inline const ::std::string& message() const;
  inline void set_message(const ::std::string& value);
  inline void set_message(const char* value);
  inline void set_message(const char* value, size_t size);
  inline ::std::string* mutable_message();
  inline ::std::string* release_message();
  inline void set_allocated_message(::std::string* message);

  // optional int32 posix_code = 4;
  inline bool has_posix_code() const;
  inline void clear_posix_code();
  static const int kPosixCodeFieldNumber = 4;
  inline ::google::protobuf::int32 posix_code() const;
  inline void set_posix_code(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:ant.AppStatusPB)
 private:
  inline void set_has_code();
  inline void clear_has_code();
  inline void set_has_message();
  inline void clear_has_message();
  inline void set_has_posix_code();
  inline void clear_has_posix_code();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* message_;
  int code_;
  ::google::protobuf::int32 posix_code_;
  friend void  protobuf_AddDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_AssignDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_ShutdownFile_ant_2fcommon_2fwire_5fprotocol_2eproto();

  void InitAsDefaultInstance();
  static AppStatusPB* default_instance_;
};
// -------------------------------------------------------------------

class NodeInstancePB : public ::google::protobuf::Message {
 public:
  NodeInstancePB();
  virtual ~NodeInstancePB();

  NodeInstancePB(const NodeInstancePB& from);

  inline NodeInstancePB& operator=(const NodeInstancePB& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const NodeInstancePB& default_instance();

  void Swap(NodeInstancePB* other);

  // implements Message ----------------------------------------------

  NodeInstancePB* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const NodeInstancePB& from);
  void MergeFrom(const NodeInstancePB& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required bytes permanent_uuid = 1;
  inline bool has_permanent_uuid() const;
  inline void clear_permanent_uuid();
  static const int kPermanentUuidFieldNumber = 1;
  inline const ::std::string& permanent_uuid() const;
  inline void set_permanent_uuid(const ::std::string& value);
  inline void set_permanent_uuid(const char* value);
  inline void set_permanent_uuid(const void* value, size_t size);
  inline ::std::string* mutable_permanent_uuid();
  inline ::std::string* release_permanent_uuid();
  inline void set_allocated_permanent_uuid(::std::string* permanent_uuid);

  // required int64 instance_seqno = 2;
  inline bool has_instance_seqno() const;
  inline void clear_instance_seqno();
  static const int kInstanceSeqnoFieldNumber = 2;
  inline ::google::protobuf::int64 instance_seqno() const;
  inline void set_instance_seqno(::google::protobuf::int64 value);

  // @@protoc_insertion_point(class_scope:ant.NodeInstancePB)
 private:
  inline void set_has_permanent_uuid();
  inline void clear_has_permanent_uuid();
  inline void set_has_instance_seqno();
  inline void clear_has_instance_seqno();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* permanent_uuid_;
  ::google::protobuf::int64 instance_seqno_;
  friend void  protobuf_AddDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_AssignDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_ShutdownFile_ant_2fcommon_2fwire_5fprotocol_2eproto();

  void InitAsDefaultInstance();
  static NodeInstancePB* default_instance_;
};
// -------------------------------------------------------------------

class ServerRegistrationPB : public ::google::protobuf::Message {
 public:
  ServerRegistrationPB();
  virtual ~ServerRegistrationPB();

  ServerRegistrationPB(const ServerRegistrationPB& from);

  inline ServerRegistrationPB& operator=(const ServerRegistrationPB& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ServerRegistrationPB& default_instance();

  void Swap(ServerRegistrationPB* other);

  // implements Message ----------------------------------------------

  ServerRegistrationPB* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ServerRegistrationPB& from);
  void MergeFrom(const ServerRegistrationPB& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .ant.HostPortPB rpc_addresses = 1;
  inline int rpc_addresses_size() const;
  inline void clear_rpc_addresses();
  static const int kRpcAddressesFieldNumber = 1;
  inline const ::ant::HostPortPB& rpc_addresses(int index) const;
  inline ::ant::HostPortPB* mutable_rpc_addresses(int index);
  inline ::ant::HostPortPB* add_rpc_addresses();
  inline const ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >&
      rpc_addresses() const;
  inline ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >*
      mutable_rpc_addresses();

  // repeated .ant.HostPortPB http_addresses = 2;
  inline int http_addresses_size() const;
  inline void clear_http_addresses();
  static const int kHttpAddressesFieldNumber = 2;
  inline const ::ant::HostPortPB& http_addresses(int index) const;
  inline ::ant::HostPortPB* mutable_http_addresses(int index);
  inline ::ant::HostPortPB* add_http_addresses();
  inline const ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >&
      http_addresses() const;
  inline ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >*
      mutable_http_addresses();

  // optional string software_version = 3;
  inline bool has_software_version() const;
  inline void clear_software_version();
  static const int kSoftwareVersionFieldNumber = 3;
  inline const ::std::string& software_version() const;
  inline void set_software_version(const ::std::string& value);
  inline void set_software_version(const char* value);
  inline void set_software_version(const char* value, size_t size);
  inline ::std::string* mutable_software_version();
  inline ::std::string* release_software_version();
  inline void set_allocated_software_version(::std::string* software_version);

  // @@protoc_insertion_point(class_scope:ant.ServerRegistrationPB)
 private:
  inline void set_has_software_version();
  inline void clear_has_software_version();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB > rpc_addresses_;
  ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB > http_addresses_;
  ::std::string* software_version_;
  friend void  protobuf_AddDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_AssignDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_ShutdownFile_ant_2fcommon_2fwire_5fprotocol_2eproto();

  void InitAsDefaultInstance();
  static ServerRegistrationPB* default_instance_;
};
// -------------------------------------------------------------------

class ServerEntryPB : public ::google::protobuf::Message {
 public:
  ServerEntryPB();
  virtual ~ServerEntryPB();

  ServerEntryPB(const ServerEntryPB& from);

  inline ServerEntryPB& operator=(const ServerEntryPB& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ServerEntryPB& default_instance();

  void Swap(ServerEntryPB* other);

  // implements Message ----------------------------------------------

  ServerEntryPB* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ServerEntryPB& from);
  void MergeFrom(const ServerEntryPB& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .ant.AppStatusPB error = 1;
  inline bool has_error() const;
  inline void clear_error();
  static const int kErrorFieldNumber = 1;
  inline const ::ant::AppStatusPB& error() const;
  inline ::ant::AppStatusPB* mutable_error();
  inline ::ant::AppStatusPB* release_error();
  inline void set_allocated_error(::ant::AppStatusPB* error);

  // optional .ant.NodeInstancePB instance_id = 2;
  inline bool has_instance_id() const;
  inline void clear_instance_id();
  static const int kInstanceIdFieldNumber = 2;
  inline const ::ant::NodeInstancePB& instance_id() const;
  inline ::ant::NodeInstancePB* mutable_instance_id();
  inline ::ant::NodeInstancePB* release_instance_id();
  inline void set_allocated_instance_id(::ant::NodeInstancePB* instance_id);

  // optional .ant.ServerRegistrationPB registration = 3;
  inline bool has_registration() const;
  inline void clear_registration();
  static const int kRegistrationFieldNumber = 3;
  inline const ::ant::ServerRegistrationPB& registration() const;
  inline ::ant::ServerRegistrationPB* mutable_registration();
  inline ::ant::ServerRegistrationPB* release_registration();
  inline void set_allocated_registration(::ant::ServerRegistrationPB* registration);

  // @@protoc_insertion_point(class_scope:ant.ServerEntryPB)
 private:
  inline void set_has_error();
  inline void clear_has_error();
  inline void set_has_instance_id();
  inline void clear_has_instance_id();
  inline void set_has_registration();
  inline void clear_has_registration();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::ant::AppStatusPB* error_;
  ::ant::NodeInstancePB* instance_id_;
  ::ant::ServerRegistrationPB* registration_;
  friend void  protobuf_AddDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_AssignDesc_ant_2fcommon_2fwire_5fprotocol_2eproto();
  friend void protobuf_ShutdownFile_ant_2fcommon_2fwire_5fprotocol_2eproto();

  void InitAsDefaultInstance();
  static ServerEntryPB* default_instance_;
};
// ===================================================================


// ===================================================================

// AppStatusPB

// required .ant.AppStatusPB.ErrorCode code = 1;
inline bool AppStatusPB::has_code() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void AppStatusPB::set_has_code() {
  _has_bits_[0] |= 0x00000001u;
}
inline void AppStatusPB::clear_has_code() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void AppStatusPB::clear_code() {
  code_ = 999;
  clear_has_code();
}
inline ::ant::AppStatusPB_ErrorCode AppStatusPB::code() const {
  // @@protoc_insertion_point(field_get:ant.AppStatusPB.code)
  return static_cast< ::ant::AppStatusPB_ErrorCode >(code_);
}
inline void AppStatusPB::set_code(::ant::AppStatusPB_ErrorCode value) {
  assert(::ant::AppStatusPB_ErrorCode_IsValid(value));
  set_has_code();
  code_ = value;
  // @@protoc_insertion_point(field_set:ant.AppStatusPB.code)
}

// optional string message = 2;
inline bool AppStatusPB::has_message() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void AppStatusPB::set_has_message() {
  _has_bits_[0] |= 0x00000002u;
}
inline void AppStatusPB::clear_has_message() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void AppStatusPB::clear_message() {
  if (message_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_->clear();
  }
  clear_has_message();
}
inline const ::std::string& AppStatusPB::message() const {
  // @@protoc_insertion_point(field_get:ant.AppStatusPB.message)
  return *message_;
}
inline void AppStatusPB::set_message(const ::std::string& value) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(value);
  // @@protoc_insertion_point(field_set:ant.AppStatusPB.message)
}
inline void AppStatusPB::set_message(const char* value) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(value);
  // @@protoc_insertion_point(field_set_char:ant.AppStatusPB.message)
}
inline void AppStatusPB::set_message(const char* value, size_t size) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:ant.AppStatusPB.message)
}
inline ::std::string* AppStatusPB::mutable_message() {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:ant.AppStatusPB.message)
  return message_;
}
inline ::std::string* AppStatusPB::release_message() {
  clear_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = message_;
    message_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void AppStatusPB::set_allocated_message(::std::string* message) {
  if (message_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete message_;
  }
  if (message) {
    set_has_message();
    message_ = message;
  } else {
    clear_has_message();
    message_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:ant.AppStatusPB.message)
}

// optional int32 posix_code = 4;
inline bool AppStatusPB::has_posix_code() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void AppStatusPB::set_has_posix_code() {
  _has_bits_[0] |= 0x00000004u;
}
inline void AppStatusPB::clear_has_posix_code() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void AppStatusPB::clear_posix_code() {
  posix_code_ = 0;
  clear_has_posix_code();
}
inline ::google::protobuf::int32 AppStatusPB::posix_code() const {
  // @@protoc_insertion_point(field_get:ant.AppStatusPB.posix_code)
  return posix_code_;
}
inline void AppStatusPB::set_posix_code(::google::protobuf::int32 value) {
  set_has_posix_code();
  posix_code_ = value;
  // @@protoc_insertion_point(field_set:ant.AppStatusPB.posix_code)
}

// -------------------------------------------------------------------

// NodeInstancePB

// required bytes permanent_uuid = 1;
inline bool NodeInstancePB::has_permanent_uuid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void NodeInstancePB::set_has_permanent_uuid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void NodeInstancePB::clear_has_permanent_uuid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void NodeInstancePB::clear_permanent_uuid() {
  if (permanent_uuid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    permanent_uuid_->clear();
  }
  clear_has_permanent_uuid();
}
inline const ::std::string& NodeInstancePB::permanent_uuid() const {
  // @@protoc_insertion_point(field_get:ant.NodeInstancePB.permanent_uuid)
  return *permanent_uuid_;
}
inline void NodeInstancePB::set_permanent_uuid(const ::std::string& value) {
  set_has_permanent_uuid();
  if (permanent_uuid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    permanent_uuid_ = new ::std::string;
  }
  permanent_uuid_->assign(value);
  // @@protoc_insertion_point(field_set:ant.NodeInstancePB.permanent_uuid)
}
inline void NodeInstancePB::set_permanent_uuid(const char* value) {
  set_has_permanent_uuid();
  if (permanent_uuid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    permanent_uuid_ = new ::std::string;
  }
  permanent_uuid_->assign(value);
  // @@protoc_insertion_point(field_set_char:ant.NodeInstancePB.permanent_uuid)
}
inline void NodeInstancePB::set_permanent_uuid(const void* value, size_t size) {
  set_has_permanent_uuid();
  if (permanent_uuid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    permanent_uuid_ = new ::std::string;
  }
  permanent_uuid_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:ant.NodeInstancePB.permanent_uuid)
}
inline ::std::string* NodeInstancePB::mutable_permanent_uuid() {
  set_has_permanent_uuid();
  if (permanent_uuid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    permanent_uuid_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:ant.NodeInstancePB.permanent_uuid)
  return permanent_uuid_;
}
inline ::std::string* NodeInstancePB::release_permanent_uuid() {
  clear_has_permanent_uuid();
  if (permanent_uuid_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = permanent_uuid_;
    permanent_uuid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NodeInstancePB::set_allocated_permanent_uuid(::std::string* permanent_uuid) {
  if (permanent_uuid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete permanent_uuid_;
  }
  if (permanent_uuid) {
    set_has_permanent_uuid();
    permanent_uuid_ = permanent_uuid;
  } else {
    clear_has_permanent_uuid();
    permanent_uuid_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:ant.NodeInstancePB.permanent_uuid)
}

// required int64 instance_seqno = 2;
inline bool NodeInstancePB::has_instance_seqno() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void NodeInstancePB::set_has_instance_seqno() {
  _has_bits_[0] |= 0x00000002u;
}
inline void NodeInstancePB::clear_has_instance_seqno() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void NodeInstancePB::clear_instance_seqno() {
  instance_seqno_ = GOOGLE_LONGLONG(0);
  clear_has_instance_seqno();
}
inline ::google::protobuf::int64 NodeInstancePB::instance_seqno() const {
  // @@protoc_insertion_point(field_get:ant.NodeInstancePB.instance_seqno)
  return instance_seqno_;
}
inline void NodeInstancePB::set_instance_seqno(::google::protobuf::int64 value) {
  set_has_instance_seqno();
  instance_seqno_ = value;
  // @@protoc_insertion_point(field_set:ant.NodeInstancePB.instance_seqno)
}

// -------------------------------------------------------------------

// ServerRegistrationPB

// repeated .ant.HostPortPB rpc_addresses = 1;
inline int ServerRegistrationPB::rpc_addresses_size() const {
  return rpc_addresses_.size();
}
inline void ServerRegistrationPB::clear_rpc_addresses() {
  rpc_addresses_.Clear();
}
inline const ::ant::HostPortPB& ServerRegistrationPB::rpc_addresses(int index) const {
  // @@protoc_insertion_point(field_get:ant.ServerRegistrationPB.rpc_addresses)
  return rpc_addresses_.Get(index);
}
inline ::ant::HostPortPB* ServerRegistrationPB::mutable_rpc_addresses(int index) {
  // @@protoc_insertion_point(field_mutable:ant.ServerRegistrationPB.rpc_addresses)
  return rpc_addresses_.Mutable(index);
}
inline ::ant::HostPortPB* ServerRegistrationPB::add_rpc_addresses() {
  // @@protoc_insertion_point(field_add:ant.ServerRegistrationPB.rpc_addresses)
  return rpc_addresses_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >&
ServerRegistrationPB::rpc_addresses() const {
  // @@protoc_insertion_point(field_list:ant.ServerRegistrationPB.rpc_addresses)
  return rpc_addresses_;
}
inline ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >*
ServerRegistrationPB::mutable_rpc_addresses() {
  // @@protoc_insertion_point(field_mutable_list:ant.ServerRegistrationPB.rpc_addresses)
  return &rpc_addresses_;
}

// repeated .ant.HostPortPB http_addresses = 2;
inline int ServerRegistrationPB::http_addresses_size() const {
  return http_addresses_.size();
}
inline void ServerRegistrationPB::clear_http_addresses() {
  http_addresses_.Clear();
}
inline const ::ant::HostPortPB& ServerRegistrationPB::http_addresses(int index) const {
  // @@protoc_insertion_point(field_get:ant.ServerRegistrationPB.http_addresses)
  return http_addresses_.Get(index);
}
inline ::ant::HostPortPB* ServerRegistrationPB::mutable_http_addresses(int index) {
  // @@protoc_insertion_point(field_mutable:ant.ServerRegistrationPB.http_addresses)
  return http_addresses_.Mutable(index);
}
inline ::ant::HostPortPB* ServerRegistrationPB::add_http_addresses() {
  // @@protoc_insertion_point(field_add:ant.ServerRegistrationPB.http_addresses)
  return http_addresses_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >&
ServerRegistrationPB::http_addresses() const {
  // @@protoc_insertion_point(field_list:ant.ServerRegistrationPB.http_addresses)
  return http_addresses_;
}
inline ::google::protobuf::RepeatedPtrField< ::ant::HostPortPB >*
ServerRegistrationPB::mutable_http_addresses() {
  // @@protoc_insertion_point(field_mutable_list:ant.ServerRegistrationPB.http_addresses)
  return &http_addresses_;
}

// optional string software_version = 3;
inline bool ServerRegistrationPB::has_software_version() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void ServerRegistrationPB::set_has_software_version() {
  _has_bits_[0] |= 0x00000004u;
}
inline void ServerRegistrationPB::clear_has_software_version() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void ServerRegistrationPB::clear_software_version() {
  if (software_version_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    software_version_->clear();
  }
  clear_has_software_version();
}
inline const ::std::string& ServerRegistrationPB::software_version() const {
  // @@protoc_insertion_point(field_get:ant.ServerRegistrationPB.software_version)
  return *software_version_;
}
inline void ServerRegistrationPB::set_software_version(const ::std::string& value) {
  set_has_software_version();
  if (software_version_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    software_version_ = new ::std::string;
  }
  software_version_->assign(value);
  // @@protoc_insertion_point(field_set:ant.ServerRegistrationPB.software_version)
}
inline void ServerRegistrationPB::set_software_version(const char* value) {
  set_has_software_version();
  if (software_version_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    software_version_ = new ::std::string;
  }
  software_version_->assign(value);
  // @@protoc_insertion_point(field_set_char:ant.ServerRegistrationPB.software_version)
}
inline void ServerRegistrationPB::set_software_version(const char* value, size_t size) {
  set_has_software_version();
  if (software_version_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    software_version_ = new ::std::string;
  }
  software_version_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:ant.ServerRegistrationPB.software_version)
}
inline ::std::string* ServerRegistrationPB::mutable_software_version() {
  set_has_software_version();
  if (software_version_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    software_version_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:ant.ServerRegistrationPB.software_version)
  return software_version_;
}
inline ::std::string* ServerRegistrationPB::release_software_version() {
  clear_has_software_version();
  if (software_version_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = software_version_;
    software_version_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void ServerRegistrationPB::set_allocated_software_version(::std::string* software_version) {
  if (software_version_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete software_version_;
  }
  if (software_version) {
    set_has_software_version();
    software_version_ = software_version;
  } else {
    clear_has_software_version();
    software_version_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:ant.ServerRegistrationPB.software_version)
}

// -------------------------------------------------------------------

// ServerEntryPB

// optional .ant.AppStatusPB error = 1;
inline bool ServerEntryPB::has_error() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ServerEntryPB::set_has_error() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ServerEntryPB::clear_has_error() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ServerEntryPB::clear_error() {
  if (error_ != NULL) error_->::ant::AppStatusPB::Clear();
  clear_has_error();
}
inline const ::ant::AppStatusPB& ServerEntryPB::error() const {
  // @@protoc_insertion_point(field_get:ant.ServerEntryPB.error)
  return error_ != NULL ? *error_ : *default_instance_->error_;
}
inline ::ant::AppStatusPB* ServerEntryPB::mutable_error() {
  set_has_error();
  if (error_ == NULL) error_ = new ::ant::AppStatusPB;
  // @@protoc_insertion_point(field_mutable:ant.ServerEntryPB.error)
  return error_;
}
inline ::ant::AppStatusPB* ServerEntryPB::release_error() {
  clear_has_error();
  ::ant::AppStatusPB* temp = error_;
  error_ = NULL;
  return temp;
}
inline void ServerEntryPB::set_allocated_error(::ant::AppStatusPB* error) {
  delete error_;
  error_ = error;
  if (error) {
    set_has_error();
  } else {
    clear_has_error();
  }
  // @@protoc_insertion_point(field_set_allocated:ant.ServerEntryPB.error)
}

// optional .ant.NodeInstancePB instance_id = 2;
inline bool ServerEntryPB::has_instance_id() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ServerEntryPB::set_has_instance_id() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ServerEntryPB::clear_has_instance_id() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ServerEntryPB::clear_instance_id() {
  if (instance_id_ != NULL) instance_id_->::ant::NodeInstancePB::Clear();
  clear_has_instance_id();
}
inline const ::ant::NodeInstancePB& ServerEntryPB::instance_id() const {
  // @@protoc_insertion_point(field_get:ant.ServerEntryPB.instance_id)
  return instance_id_ != NULL ? *instance_id_ : *default_instance_->instance_id_;
}
inline ::ant::NodeInstancePB* ServerEntryPB::mutable_instance_id() {
  set_has_instance_id();
  if (instance_id_ == NULL) instance_id_ = new ::ant::NodeInstancePB;
  // @@protoc_insertion_point(field_mutable:ant.ServerEntryPB.instance_id)
  return instance_id_;
}
inline ::ant::NodeInstancePB* ServerEntryPB::release_instance_id() {
  clear_has_instance_id();
  ::ant::NodeInstancePB* temp = instance_id_;
  instance_id_ = NULL;
  return temp;
}
inline void ServerEntryPB::set_allocated_instance_id(::ant::NodeInstancePB* instance_id) {
  delete instance_id_;
  instance_id_ = instance_id;
  if (instance_id) {
    set_has_instance_id();
  } else {
    clear_has_instance_id();
  }
  // @@protoc_insertion_point(field_set_allocated:ant.ServerEntryPB.instance_id)
}

// optional .ant.ServerRegistrationPB registration = 3;
inline bool ServerEntryPB::has_registration() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void ServerEntryPB::set_has_registration() {
  _has_bits_[0] |= 0x00000004u;
}
inline void ServerEntryPB::clear_has_registration() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void ServerEntryPB::clear_registration() {
  if (registration_ != NULL) registration_->::ant::ServerRegistrationPB::Clear();
  clear_has_registration();
}
inline const ::ant::ServerRegistrationPB& ServerEntryPB::registration() const {
  // @@protoc_insertion_point(field_get:ant.ServerEntryPB.registration)
  return registration_ != NULL ? *registration_ : *default_instance_->registration_;
}
inline ::ant::ServerRegistrationPB* ServerEntryPB::mutable_registration() {
  set_has_registration();
  if (registration_ == NULL) registration_ = new ::ant::ServerRegistrationPB;
  // @@protoc_insertion_point(field_mutable:ant.ServerEntryPB.registration)
  return registration_;
}
inline ::ant::ServerRegistrationPB* ServerEntryPB::release_registration() {
  clear_has_registration();
  ::ant::ServerRegistrationPB* temp = registration_;
  registration_ = NULL;
  return temp;
}
inline void ServerEntryPB::set_allocated_registration(::ant::ServerRegistrationPB* registration) {
  delete registration_;
  registration_ = registration;
  if (registration) {
    set_has_registration();
  } else {
    clear_has_registration();
  }
  // @@protoc_insertion_point(field_set_allocated:ant.ServerEntryPB.registration)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace ant

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::ant::AppStatusPB_ErrorCode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::ant::AppStatusPB_ErrorCode>() {
  return ::ant::AppStatusPB_ErrorCode_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_ant_2fcommon_2fwire_5fprotocol_2eproto__INCLUDED
