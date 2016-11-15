#ifndef KUDU_RPC_NEGOTIATION_H
#define KUDU_RPC_NEGOTIATION_H

#include "ant/base/ref_counted.h"
#include "ant/util/monotime.h"

namespace ant {
namespace rpc {

class Connection;

class Negotiation {
 public:
  static void RunNegotiation(const scoped_refptr<Connection>& conn,
                             const MonoTime &deadline);
 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Negotiation);
};

} // namespace rpc
} // namespace ant
#endif // KUDU_RPC_NEGOTIATION_H
