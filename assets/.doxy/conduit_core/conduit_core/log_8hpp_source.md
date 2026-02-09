

# File log.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**log.hpp**](log_8hpp.md)

[Go to the documentation of this file](log_8hpp.md)


```C++
#pragma once

#include <string_view>
#include <fmt/format.h>

namespace conduit::log {

enum class Level {
    Debug,   
    Info,    
    Warn,    
    Error    
};

void set_level(Level level);

Level get_level();

void debug(std::string_view msg);

void info(std::string_view msg);

void warn(std::string_view msg);

void error(std::string_view msg);

template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    debug(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args) {
    info(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void warn(fmt::format_string<Args...> fmt, Args&&... args) {
    warn(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void error(fmt::format_string<Args...> fmt, Args&&... args) {
    error(fmt::format(fmt, std::forward<Args>(args)...));
}

}  // namespace conduit::log
```


