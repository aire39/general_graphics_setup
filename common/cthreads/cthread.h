#pragma once

#include <thread>
#include <string>
#include <functional>

class cthread : public std::thread
{
  public:
    cthread() : std::thread()
    {
    }

    template<typename callable, typename... argslist>
    explicit cthread(callable&& func, argslist&&... args)
      : std::thread
      (&cthread::cinit
      ,this
      ,std::forward<std::string>("Unnamed")
      ,std::forward<std::string>("No description")
      ,std::bind(std::forward<callable>(func) ,std::forward<argslist>(args)...)
      )
    {
    }

    template<typename callable, typename... argslist>
    explicit cthread(std::string && thread_name, callable&& func, argslist&&... args)
      : std::thread
      (&cthread::cinit
      ,this
      ,std::forward<std::string>(thread_name)
      ,std::forward<std::string>("No description")
      ,std::bind(std::forward<callable>(func), std::forward<argslist>(args)...)
      )
      ,threadName(thread_name)
    {
    }

    template<typename callable, typename... argslist>
    explicit cthread(const char * thread_name, callable&& func, argslist&&... args)
      : std::thread
      (&cthread::cinit
      ,this
      ,std::forward<std::string>(thread_name)
      ,std::forward<std::string>("No description")
      ,std::bind(std::forward<callable>(func), std::forward<argslist>(args)...)
      )
      ,threadName(thread_name)
    {
    }

    template<typename callable, typename... argslist>
    explicit cthread(std::string && thread_name, std::string && thread_description, callable&& func, argslist&&... args)
      : std::thread
      (&cthread::cinit
      ,this
      ,std::forward<std::string>(thread_name)
      ,std::forward<std::string>(thread_description)
      ,std::bind(std::forward<callable>(func), std::forward<argslist>(args)...)
      )
      , threadName(thread_name)
      , threadDesc(thread_description)
    {
    }

    template<typename callable, typename... argslist>
    explicit cthread(const char * thread_name, const char * thread_description, callable&& func, argslist&&... args)
      : std::thread
      (&cthread::cinit
      ,this
      ,std::forward<std::string>(thread_name)
      ,std::forward<std::string>(thread_description)
      ,std::bind(std::forward<callable>(func), std::forward<argslist>(args)...)
      )
      ,threadName(thread_name)
      ,threadDesc(thread_description)
    {
    }

    void setname(std::string &name);
    void setdescription(const std::string &description);

    std::string name();
    std::string description();

  private:
    void cinit(std::string && thread_name, std::string && thread_description, std::function<void()> && thread_task) noexcept;

    std::string threadName = "Unnamed";
    std::string threadDesc = "No description";
};
