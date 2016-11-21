#ifndef ANT_COMMON_WIRE_PROTOCOL_H_
#define ANT_COMMON_WIRE_PROTOCOL_H_
#include <vector>

#include "ant/common/wire_protocol.pb.h"
#include "ant/util/status.h"

namespace ant {

class Arena;
class faststring;
class HostPort;
class Slice;
class Sockaddr;

void StatusToPB(const Status& status, AppStatusPB* pb);
Status StatusFromPB(const AppStatusPB& pb);
Status HostPortToPB(const HostPort& host_port, HostPortPB* host_port_pb);
Status HostPortFromPB(const HostPortPB& host_port_pb, HostPort* host_port);
Status AddHostPortPBs(const std::vector<Sockaddr>& addrs,
		      google::protobuf::RepeatedPtrField<HostPortPB>* pbs);

} // namespace ant
#endif // ANT_COMMON_WIRE_PROTOCOL_H_
