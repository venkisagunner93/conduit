

# Class conduit::internal::Publisher



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**Publisher**](classconduit_1_1internal_1_1Publisher.md)



_Low-level publisher that writes raw bytes to a shared memory ring buffer._ [More...](#detailed-description)

* `#include <publisher.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Publisher**](#function-publisher-13) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & topic, [**const**](classconduit_1_1internal_1_1Publisher.md) [**PublisherOptions**](structconduit_1_1PublisherOptions.md) & options={}) <br>_Construct a publisher for the given topic._  |
|   | [**Publisher**](#function-publisher-23) ([**Publisher**](classconduit_1_1internal_1_1Publisher.md) &&) noexcept<br>_Move constructor._  |
|   | [**Publisher**](#function-publisher-33) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**Publisher**](classconduit_1_1internal_1_1Publisher.md) &) = delete<br> |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**max\_message\_size**](#function-max_message_size) () const<br>_Get the maximum allowed message size._  |
|  [**Publisher**](classconduit_1_1internal_1_1Publisher.md) & | [**operator=**](#function-operator) ([**Publisher**](classconduit_1_1internal_1_1Publisher.md) &&) noexcept<br>_Move assignment operator._  |
|  [**Publisher**](classconduit_1_1internal_1_1Publisher.md) & | [**operator=**](#function-operator_1) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**Publisher**](classconduit_1_1internal_1_1Publisher.md) &) = delete<br> |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**publish**](#function-publish) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**void**](classconduit_1_1internal_1_1Publisher.md) \* data, [**size\_t**](classconduit_1_1internal_1_1Publisher.md) size) <br>_Publish raw data to the topic._  |
|  [**const**](classconduit_1_1internal_1_1Publisher.md) std::string & | [**topic**](#function-topic) () const<br>_Get the topic name._  |
|   | [**~Publisher**](#function-publisher) () <br> |




























## Detailed Description


Creates a shared memory region at `/dev/shm/conduit_{topic}` and manages a lock-free SPMC ring buffer for zero-copy message delivery. Use the typed Publisher&lt;T&gt; wrapper for type-safe publishing.




**See also:** conduit::Publisher 



    
## Public Functions Documentation




### function Publisher [1/3]

_Construct a publisher for the given topic._ 
```C++
conduit::internal::Publisher::Publisher (
    const std::string & topic,
    const  PublisherOptions & options={}
) 
```





**Parameters:**


* `topic` Topic name used to create the shared memory region. 
* `options` Ring buffer configuration (depth and max message size). 



**Exception:**


* [**PublisherError**](classconduit_1_1PublisherError.md) If shared memory creation fails. 




        

<hr>



### function Publisher [2/3]

_Move constructor._ 
```C++
conduit::internal::Publisher::Publisher (
    Publisher &&
) noexcept
```




<hr>



### function Publisher [3/3]

```C++
conduit::internal::Publisher::Publisher (
    const  Publisher &
) = delete
```




<hr>



### function max\_message\_size 

_Get the maximum allowed message size._ 
```C++
inline uint32_t conduit::internal::Publisher::max_message_size () const
```





**Returns:**

Maximum payload size in bytes. 





        

<hr>



### function operator= 

_Move assignment operator._ 
```C++
Publisher & conduit::internal::Publisher::operator= (
    Publisher &&
) noexcept
```




<hr>



### function operator= 

```C++
Publisher & conduit::internal::Publisher::operator= (
    const  Publisher &
) = delete
```




<hr>



### function publish 

_Publish raw data to the topic._ 
```C++
bool conduit::internal::Publisher::publish (
    const  void * data,
    size_t size
) 
```





**Parameters:**


* `data` Pointer to the payload bytes. 
* `size` Size of the payload in bytes. 



**Returns:**

true if the message was written, false if size exceeds max\_message\_size. 





        

<hr>



### function topic 

_Get the topic name._ 
```C++
inline const std::string & conduit::internal::Publisher::topic () const
```





**Returns:**

Reference to the topic string. 





        

<hr>



### function ~Publisher 

```C++
conduit::internal::Publisher::~Publisher () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/publisher.hpp`

