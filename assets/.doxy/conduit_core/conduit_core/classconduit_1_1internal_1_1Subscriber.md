

# Class conduit::internal::Subscriber



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md)



_Low-level subscriber that reads raw bytes from a shared memory ring buffer._ [More...](#detailed-description)

* `#include <subscriber.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Subscriber**](#function-subscriber-13) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & topic, [**const**](classconduit_1_1internal_1_1Publisher.md) [**SubscriberOptions**](structconduit_1_1SubscriberOptions.md) & options={}) <br>_Construct a subscriber for the given topic._  |
|   | [**Subscriber**](#function-subscriber-23) ([**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) &&) noexcept<br>_Move constructor._  |
|   | [**Subscriber**](#function-subscriber-33) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) &) = delete<br> |
|  [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) & | [**operator=**](#function-operator) ([**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) &&) noexcept<br>_Move assignment operator._  |
|  [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) & | [**operator=**](#function-operator_1) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) &) = delete<br> |
|  std::optional&lt; [**Message**](structconduit_1_1Message.md) &gt; | [**take**](#function-take) () <br>_Non-blocking read of the next message._  |
|  [**const**](classconduit_1_1internal_1_1Publisher.md) std::string & | [**topic**](#function-topic) () const<br>_Get the topic name._  |
|  [**Message**](structconduit_1_1Message.md) | [**wait**](#function-wait) () <br>_Block until a message is available._  |
|  std::optional&lt; [**Message**](structconduit_1_1Message.md) &gt; | [**wait\_for**](#function-wait_for) (std::chrono::nanoseconds timeout) <br>_Block until a message is available or timeout expires._  |
|   | [**~Subscriber**](#function-subscriber) () <br> |




























## Detailed Description


Opens an existing shared memory region and claims a reader slot in the lock-free SPMC ring buffer. Use the typed Subscriber&lt;T&gt; wrapper for automatic deserialization.




**See also:** conduit::Subscriber 



    
## Public Functions Documentation




### function Subscriber [1/3]

_Construct a subscriber for the given topic._ 
```C++
conduit::internal::Subscriber::Subscriber (
    const std::string & topic,
    const  SubscriberOptions & options={}
) 
```





**Parameters:**


* `topic` Topic name of the shared memory region to open. 
* `options` [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) configuration (reserved for future use). 



**Exception:**


* [**SubscriberError**](classconduit_1_1SubscriberError.md) If shared memory cannot be opened or no reader slots available. 




        

<hr>



### function Subscriber [2/3]

_Move constructor._ 
```C++
conduit::internal::Subscriber::Subscriber (
    Subscriber &&
) noexcept
```




<hr>



### function Subscriber [3/3]

```C++
conduit::internal::Subscriber::Subscriber (
    const  Subscriber &
) = delete
```




<hr>



### function operator= 

_Move assignment operator._ 
```C++
Subscriber & conduit::internal::Subscriber::operator= (
    Subscriber &&
) noexcept
```




<hr>



### function operator= 

```C++
Subscriber & conduit::internal::Subscriber::operator= (
    const  Subscriber &
) = delete
```




<hr>



### function take 

_Non-blocking read of the next message._ 
```C++
std::optional< Message > conduit::internal::Subscriber::take () 
```





**Returns:**

The next message, or std::nullopt if no new message is available. 





        

<hr>



### function topic 

_Get the topic name._ 
```C++
inline const std::string & conduit::internal::Subscriber::topic () const
```





**Returns:**

Reference to the topic string. 





        

<hr>



### function wait 

_Block until a message is available._ 
```C++
Message conduit::internal::Subscriber::wait () 
```



Uses futex-based signaling for zero CPU usage while idle.




**Returns:**

The next message. 





        

<hr>



### function wait\_for 

_Block until a message is available or timeout expires._ 
```C++
std::optional< Message > conduit::internal::Subscriber::wait_for (
    std::chrono::nanoseconds timeout
) 
```





**Parameters:**


* `timeout` Maximum time to wait. 



**Returns:**

The next message, or std::nullopt on timeout. 





        

<hr>



### function ~Subscriber 

```C++
conduit::internal::Subscriber::~Subscriber () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/subscriber.hpp`

