

# Class conduit::Publisher

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**Publisher**](classconduit_1_1Publisher.md)



_Type-safe publisher that serializes messages of type T._ [More...](#detailed-description)






































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Publisher**](#function-publisher-13) (const std::string & topic, const [**PublisherOptions**](structconduit_1_1PublisherOptions.md) & options={}) <br>_Construct a typed publisher for the given topic._  |
|   | [**Publisher**](#function-publisher-23) (Publisher &&) noexcept<br>_Move constructor._  |
|   | [**Publisher**](#function-publisher-33) (const Publisher &) = delete<br> |
|  uint32\_t | [**max\_message\_size**](#function-max_message_size) () const<br>_Get the maximum allowed message size._  |
|  Publisher & | [**operator=**](#function-operator) (Publisher &&) noexcept<br>_Move assignment operator._  |
|  Publisher & | [**operator=**](#function-operator_1) (const Publisher &) = delete<br> |
|  bool | [**publish**](#function-publish) (const T & msg) <br>_Publish a typed message._  |
|  const std::string & | [**topic**](#function-topic) () const<br>_Get the topic name._  |




























## Detailed Description


For FixedMessageType derivatives, messages are published via memcpy. For VariableMessageType derivatives, serialize() is called into an internal buffer before publishing.




**Template parameters:**


* `T` [**Message**](structconduit_1_1Message.md) type (must derive from FixedMessageType or VariableMessageType). 



**See also:** [**PublisherOptions**](structconduit_1_1PublisherOptions.md), [**Node::advertise**](classconduit_1_1Node.md#function-advertise) 



    
## Public Functions Documentation




### function Publisher [1/3]

_Construct a typed publisher for the given topic._ 
```C++
inline Publisher::Publisher (
    const std::string & topic,
    const PublisherOptions & options={}
) 
```





**Parameters:**


* `topic` Topic name used to create the shared memory region. 
* `options` Ring buffer configuration. 




        

<hr>



### function Publisher [2/3]

_Move constructor._ 
```C++
Publisher::Publisher (
    Publisher &&
) noexcept
```




<hr>



### function Publisher [3/3]

```C++
Publisher::Publisher (
    const Publisher &
) = delete
```




<hr>



### function max\_message\_size 

_Get the maximum allowed message size._ 
```C++
inline uint32_t Publisher::max_message_size () const
```





**Returns:**

Maximum payload size in bytes. 





        

<hr>



### function operator= 

_Move assignment operator._ 
```C++
Publisher & Publisher::operator= (
    Publisher &&
) noexcept
```




<hr>



### function operator= 

```C++
Publisher & Publisher::operator= (
    const Publisher &
) = delete
```




<hr>



### function publish 

_Publish a typed message._ 
```C++
inline bool Publisher::publish (
    const T & msg
) 
```



Fixed types are published via memcpy. Variable types are serialized into an internal buffer first.




**Parameters:**


* `msg` The message to publish. 



**Returns:**

true if the message was written, false if it exceeds max\_message\_size. 





        

<hr>



### function topic 

_Get the topic name._ 
```C++
inline const std::string & Publisher::topic () const
```





**Returns:**

Reference to the topic string. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/publisher.hpp`

