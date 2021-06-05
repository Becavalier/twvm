#include "lib/instantiator.h"

namespace TWVM {
  const shared_module_instance_t Instantiator::instantiate(shared_module_t mod) {
    return std::make_shared<Instance>();
    // 
  }
}