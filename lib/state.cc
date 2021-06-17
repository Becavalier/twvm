#include <atomic>
#include <mutex>
#include "lib/include/state.hh"

namespace TWVM {
  std::atomic<State*> State::instance = nullptr;
  std::mutex State::mutex;
  State* State::getGlobalState() {
    State* state = instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (state == nullptr) {
      std::lock_guard<std::mutex> lock(mutex);
      state = instance.load(std::memory_order_relaxed);
      if (state == nullptr) {
        state = new State(); 
        std::atomic_thread_fence(std::memory_order_release);
        instance.store(state, std::memory_order_relaxed);
      }
    }
    return state;
  }
}
 