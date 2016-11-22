#ifndef ANT_MASTER_MASTER_OPTIONS_H_
#define ANT_MASTER_MASTER_OPTIONS_H_

#include <vector>

#include "ant/server/server_base_options.h"
#include "ant/util/net/net_util.h"

namespace ant {
namespace master {

struct MasterOptions : public server::ServerBaseOptions {
  MasterOptions();

  std::vector<HostPort> master_addresses;

  bool IsDistributed() const;
};

} // namespace master
} // namespace ant
#endif // ANT_MASTER_MASTER_OPTIONS_H_
