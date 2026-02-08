

# Class conduit::Subscriber

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**Subscriber**](classconduit_1_1Subscriber.md)



_Type-safe subscriber that deserializes messages of type T._ [More...](#detailed-description)






































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Subscriber**](#function-subscriber-13) (const std::string & topic, const [**SubscriberOptions**](structconduit_1_1SubscriberOptions.md) & options={}) <br>_Construct a typed subscriber for the given topic._  |
|   | [**Subscriber**](#function-subscriber-23) (Subscriber &&) noexcept<br>_Move constructor._  |
|   | [**Subscriber**](#function-subscriber-33) (const Subscriber &) = delete<br> |
|  Subscriber & | [**operator=**](#function-operator) (Subscriber &&) noexcept<br>_Move assignment operator._  |
|  Subscriber & | [**operator=**](#function-operator_1) (const Subscriber &) = delete<br> |
|  std::optional&lt; TypedMessage&lt; T &gt; &gt; | [**take**](#function-take) () <br>_Non-blocking read of the next typed message._  |
|  const std::string & | [**topic**](#function-topic) () const<br>_Get the topic name._  |
|  TypedMessage&lt; T &gt; | [**wait**](#function-wait) () <br>_Block until a typed message is available._  |
|  std::optional&lt; TypedMessage&lt; T &gt; &gt; | [**wait\_for**](#function-wait_for) (std::chrono::nanoseconds timeout) <br>_Block until a typed message is available or timeout expires._  |




























## Detailed Description


For FixedMessageType derivatives, messages are deserialized via memcpy. For VariableMessageType derivatives, T::deserialize() is called.




**Template parameters:**


* `T` [**Message**](structconduit_1_1Message.md) type (must derive from FixedMessageType or VariableMessageType). 



**See also:** [**SubscriberOptions**](structconduit_1_1SubscriberOptions.md), [**Node::subscribe**](classconduit_1_1Node.md#function-subscribe-13) 



    
## Public Functions Documentation




### function Subscriber [1/3]

_Construct a typed subscriber for the given topic._ 
```C++
inline Subscriber::Subscriber (
    const std::string & topic,
    const SubscriberOptions & options={}
) 
```





**Parameters:**


* `topic` Topic name of the shared memory region to open. 
* `options` Subscriber configuration. 




        

<hr>



### function Subscriber [2/3]

_Move constructor._ 
```C++
Subscriber::Subscriber (
    Subscriber &&
) noexcept
```




<hr>



### function Subscriber [3/3]

```C++
Subscriber::Subscriber (
    const Subscriber &
) = delete
```




<hr>



### function operator= 

_Move assignment operator._ 
```C++
Subscriber & Subscriber::operator= (
    Subscriber &&
) noexcept
```




<hr>



### function operator= 

```C++
Subscriber & Subscriber::operator= (
    const Subscriber &
) = delete
```




<hr>



### function take 

_Non-blocking read of the next typed message._ 
```C++
inline std::optional< TypedMessage< T > > Subscriber::take () 
```





**Returns:**

Deserialized message, or std::nullopt if no new message is available. 





        

<hr>



### function topic 

_Get the topic name._ 
```C++
inline const std::string & Subscriber::topic () const
```





**Returns:**

Reference to the topic string. 





        

<hr>



### function wait 

_Block until a typed message is available._ 
```C++
inline TypedMessage< T > Subscriber::wait () 
```





**Returns:**

The next deserialized message. 





        

<hr>



### function wait\_for 

_Block until a typed message is available or timeout expires._ 
```C++
inline std::optional< TypedMessage< T > > Subscriber::wait_for (
    std::chrono::nanoseconds timeout
) 
```





**Parameters:**


* `timeout` Maximum time to wait. 



**Returns:**

Deserialized message, or std::nullopt on timeout. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/subscriber.hpp`

