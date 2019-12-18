#include <memory>
#include "gtest/gtest.h"
#include "src/twvm.h"

using std::make_unique;

TEST(TWVM, MemoryOperations) {
  const auto executor = make_unique<Executor>();

  executor->execute((
    Instantiator::instantiate(
      Loader::init()));
}
