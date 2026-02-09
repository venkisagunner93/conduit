

# Namespace conduit::log



[**Namespace List**](namespaces.md) **>** [**conduit**](namespaceconduit.md) **>** [**log**](namespaceconduit_1_1log.md)



_Conduit logging utilities._ 


















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Level**](#enum-level)  <br>_Log severity levels._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**debug**](#function-debug) (std::string\_view msg) <br>_Log a debug message._  |
|  void | [**debug**](#function-debug) (fmt::format\_string&lt; Args... &gt; fmt, Args &&... args) <br>_Log a formatted debug message._  |
|  void | [**error**](#function-error) (std::string\_view msg) <br>_Log an error message._  |
|  void | [**error**](#function-error) (fmt::format\_string&lt; Args... &gt; fmt, Args &&... args) <br>_Log a formatted error message._  |
|  [**Level**](namespaceconduit_1_1log.md#enum-level) | [**get\_level**](#function-get_level) () <br>_Get the current global log level._  |
|  void | [**info**](#function-info) (std::string\_view msg) <br>_Log an informational message._  |
|  void | [**info**](#function-info) (fmt::format\_string&lt; Args... &gt; fmt, Args &&... args) <br>_Log a formatted informational message._  |
|  void | [**set\_level**](#function-set_level) ([**Level**](namespaceconduit_1_1log.md#enum-level) level) <br>_Set the global log level. Messages below this level are suppressed._  |
|  void | [**warn**](#function-warn) (std::string\_view msg) <br>_Log a warning message._  |
|  void | [**warn**](#function-warn) (fmt::format\_string&lt; Args... &gt; fmt, Args &&... args) <br>_Log a formatted warning message._  |




























## Public Types Documentation




### enum Level 

_Log severity levels._ 
```C++
enum conduit::log::Level {
    Debug,
    Info,
    Warn,
    Error
};
```




<hr>
## Public Functions Documentation




### function debug 

_Log a debug message._ 
```C++
void conduit::log::debug (
    std::string_view msg
) 
```





**Parameters:**


* `msg` [**Message**](structconduit_1_1Message.md) string. 




        

<hr>



### function debug 

_Log a formatted debug message._ 
```C++
template<typename... Args>
void conduit::log::debug (
    fmt::format_string< Args... > fmt,
    Args &&... args
) 
```





**Template parameters:**


* `Args` Format argument types. 



**Parameters:**


* `fmt` Format string (fmt library syntax). 
* `args` Format arguments. 




        

<hr>



### function error 

_Log an error message._ 
```C++
void conduit::log::error (
    std::string_view msg
) 
```





**Parameters:**


* `msg` [**Message**](structconduit_1_1Message.md) string. 




        

<hr>



### function error 

_Log a formatted error message._ 
```C++
template<typename... Args>
void conduit::log::error (
    fmt::format_string< Args... > fmt,
    Args &&... args
) 
```





**Template parameters:**


* `Args` Format argument types. 



**Parameters:**


* `fmt` Format string (fmt library syntax). 
* `args` Format arguments. 




        

<hr>



### function get\_level 

_Get the current global log level._ 
```C++
Level conduit::log::get_level () 
```





**Returns:**

The active log level. 





        

<hr>



### function info 

_Log an informational message._ 
```C++
void conduit::log::info (
    std::string_view msg
) 
```





**Parameters:**


* `msg` [**Message**](structconduit_1_1Message.md) string. 




        

<hr>



### function info 

_Log a formatted informational message._ 
```C++
template<typename... Args>
void conduit::log::info (
    fmt::format_string< Args... > fmt,
    Args &&... args
) 
```





**Template parameters:**


* `Args` Format argument types. 



**Parameters:**


* `fmt` Format string (fmt library syntax). 
* `args` Format arguments. 




        

<hr>



### function set\_level 

_Set the global log level. Messages below this level are suppressed._ 
```C++
void conduit::log::set_level (
    Level level
) 
```





**Parameters:**


* `level` The minimum severity level to output. 




        

<hr>



### function warn 

_Log a warning message._ 
```C++
void conduit::log::warn (
    std::string_view msg
) 
```





**Parameters:**


* `msg` [**Message**](structconduit_1_1Message.md) string. 




        

<hr>



### function warn 

_Log a formatted warning message._ 
```C++
template<typename... Args>
void conduit::log::warn (
    fmt::format_string< Args... > fmt,
    Args &&... args
) 
```





**Template parameters:**


* `Args` Format argument types. 



**Parameters:**


* `fmt` Format string (fmt library syntax). 
* `args` Format arguments. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/log.hpp`

