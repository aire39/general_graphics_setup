#pragma once

#include <algorithm>
#include <variant>

#if __cpp_lib_parallel_algorithm || __cpp_lib_execution && __cpp_lib_execution >= 201603 || defined(__GNUC__) && (__GNUC__ > 9 || (__GNUC__ == 9 && __GNUC_MINOR__ >= 0)) || _MSC_VER >= 1926
#include <execution>
#else
#include <tbb/parallel_for_each.h>
#include <tbb/task_scheduler_observer.h>
#endif

#include "graphics/FilterTypes.h"

namespace gss::cprocess::loops {

  template <class ForwardIt, class Callback>
  void for_each([[maybe_unused]] const filter::types::ExecutionPolicies policies, ForwardIt data_begin, ForwardIt data_end, Callback callback)
  {
#if __cpp_lib_parallel_algorithm || __cpp_lib_execution && __cpp_lib_execution >= 201603 || defined(__GNUC__) && (__GNUC__ > 9 || (__GNUC__ == 9 && __GNUC_MINOR__ >= 0)) || _MSC_VER >= 1926
    std::variant<std::execution::sequenced_policy, std::execution::unsequenced_policy, std::execution::parallel_policy, std::execution::parallel_unsequenced_policy> v_policies;

    switch (policies)
    {
      case filter::types::ExecutionPolicies::par_unseq:
        v_policies = std::execution::par_unseq;
        break;

      case filter::types::ExecutionPolicies::par:
        v_policies = std::execution::par;
        break;

      case filter::types::ExecutionPolicies::unseq:
        v_policies = std::execution::unseq;
        break;

      case filter::types::ExecutionPolicies::seq:
      default:
        v_policies = std::execution::seq;
        break;
    }

    std::visit([&](auto policy) {
    std::for_each(policy, data_begin, data_end, callback);
    }, v_policies);

#else

    static bool warm_up_tbb = false;
    if (!warm_up_tbb)
    {
      warm_up_tbb = true;
      tbb::parallel_for(0, tbb::this_task_arena::max_concurrency(), [](int) {});
    }

    if (policies == filter::types::ExecutionPolicies::par_unseq)
    {
      const uint64_t size = data_end - data_begin;
      constexpr uint64_t start = 0;
      tbb::parallel_for(tbb::blocked_range(start, size), [&](const tbb::blocked_range<uint64_t>& r) {
          for (uint64_t i = r.begin(); i != r.end(); ++i)
          {
            callback(*(data_begin + i));
          }
        }
      );
    }
    else if (policies == filter::types::ExecutionPolicies::par)
    {
      tbb::parallel_for_each(data_begin, data_end, callback);
    }
    else // seq and unseq
    {
      std::for_each(data_begin, data_end, callback);
    }

#endif
  }
}
