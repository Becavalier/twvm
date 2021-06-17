// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_STATE_H_
#define LIB_STATE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include "lib/include/exception.hh"

namespace TWVM {
  class State {
    class Item {
      const std::string v;
     public:
      Item(std::string& str) : v(str) {}
      auto& toStr() const { return v; }
      auto toBool() const { return v == "true"; }
      auto toInt() const { 
        try {
          return std::stoi(v); 
        } catch (...) {
          Exception::terminate(Exception::ErrorType::INVALID_CONVERSION_STOI);
        }
      }
    };
    State() = default;
    State(const State&) = delete;
    static std::atomic<State*> instance;
    static std::mutex mutex;
    std::unordered_map<std::string, Item> store; 
    static State* getGlobalState();
   public:
    static std::optional<const Item*> retrieveItem(const char* key) {
      const auto& store = getGlobalState()->store;
      const auto& item = store.find(key);
      return item != store.end() ? std::make_optional(&item->second) : std::nullopt;
    }
    static void createItem(std::string key, std::string v) {
      auto& store = getGlobalState()->store; 
      {
        std::lock_guard<std::mutex> lock(State::mutex);
        store.emplace(std::make_pair(std::ref(key), std::ref(v)));
      }
    }
  };
}

#endif  // LIB_STATE_H_
