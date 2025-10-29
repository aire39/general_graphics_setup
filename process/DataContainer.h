#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include "GenericData.h"

class DataContainer
{
  public:
    template <std::derived_from<GenericData> T>
    void Add(std::shared_ptr<T> data)
    {
      collection.insert({typeid(*data.get()), data});
    }

    template <std::derived_from<GenericData> T>
    std::shared_ptr<T> GetData()
    {
      std::shared_ptr<T> found_data = nullptr;

      if (auto it = collection.find(typeid(T)); it != collection.end())
      {
        std::shared_ptr<GenericData> data = it->second;
        found_data = std::static_pointer_cast<T>(it->second);
      }
      return found_data;
    }

    bool IsDataContainerEmpty() const
    {
      return collection.empty();
    }

  private:
    std::unordered_map<std::type_index, std::shared_ptr<GenericData>> collection;
};
