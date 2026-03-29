

# Class conduit::Node



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Node**](classconduit_1_1Node.md)



_Base class for conduit processing nodes._ [More...](#detailed-description)

* `#include <node.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Node**](#function-node-13) () <br>_Construct a node._  |
|   | [**Node**](#function-node-23) (const [**Node**](classconduit_1_1Node.md) &) = delete<br> |
|   | [**Node**](#function-node-33) ([**Node**](classconduit_1_1Node.md) &&) = delete<br> |
|  [**Node**](classconduit_1_1Node.md) & | [**operator=**](#function-operator) (const [**Node**](classconduit_1_1Node.md) &) = delete<br> |
|  [**Node**](classconduit_1_1Node.md) & | [**operator=**](#function-operator_1) ([**Node**](classconduit_1_1Node.md) &&) = delete<br> |
|  void | [**run**](#function-run) () <br>_Run the node, blocking until SIGINT/SIGTERM or_ [_**stop()**_](classconduit_1_1Node.md#function-stop) _is called._ |
|  bool | [**running**](#function-running) () const<br>_Check if the node is currently running._  |
|  void | [**stop**](#function-stop) () <br>_Stop the node (can be called from any thread or signal handler)._  |
| virtual  | [**~Node**](#function-node) () <br> |
























## Protected Functions

| Type | Name |
| ---: | :--- |
|  Publisher&lt; T &gt; | [**advertise**](#function-advertise) (const std::string & topic, const [**PublisherOptions**](structconduit_1_1PublisherOptions.md) & options={}) <br>_Create a typed publisher for the given topic._  |
|  void | [**loop**](#function-loop-12) (double rate\_hz, Func T::\* callback) <br>_Register a fixed-rate loop with a member function callback._  |
|  void | [**loop**](#function-loop-22) (double rate\_hz, std::function&lt; void()&gt; callback) <br>_Register a fixed-rate loop with a lambda or std::function callback._  |
|  void | [**subscribe**](#function-subscribe-13) (const std::string & topic, Func T::\* callback) <br>_Subscribe to a topic with a member function callback (raw)._  |
|  void | [**subscribe**](#function-subscribe-23) (const std::string & topic, void(T::\*)(const TypedMessage&lt; MsgT &gt; &) callback) <br>_Subscribe to a topic with a typed member function callback._  |
|  void | [**subscribe**](#function-subscribe-33) (const std::string & topic, std::function&lt; void(const [**Message**](structconduit_1_1Message.md) &)&gt; callback) <br>_Subscribe to a topic with a lambda or std::function callback (raw)._  |




## Detailed Description


A [**Node**](classconduit_1_1Node.md) manages subscriptions and publish loops. Each subscription runs on its own thread, with callbacks dispatched automatically when messages arrive. Call [**run()**](classconduit_1_1Node.md#function-run) to start all threads and block until SIGINT/SIGTERM or [**stop()**](classconduit_1_1Node.md#function-stop) is called.



```C++
class MyNode : public conduit::Node {
public:
    MyNode() {
        subscribe<Imu>("imu", &MyNode::on_imu);
        pub_ = advertise<Twist>("cmd_vel");
        loop(10.0, &MyNode::tick);
    }
private:
    void on_imu(const TypedMessage<Imu>& msg) { /* ... */ }
    void tick() { pub_.publish(Twist{}); }
    Publisher<Twist> pub_;
};
```





**See also:** Publisher, Subscriber 



    
## Public Functions Documentation




### function Node [1/3]

_Construct a node._ 
```C++
conduit::Node::Node () 
```




<hr>



### function Node [2/3]

```C++
conduit::Node::Node (
    const Node &
) = delete
```




<hr>



### function Node [3/3]

```C++
conduit::Node::Node (
    Node &&
) = delete
```




<hr>



### function operator= 

```C++
Node & conduit::Node::operator= (
    const Node &
) = delete
```




<hr>



### function operator= 

```C++
Node & conduit::Node::operator= (
    Node &&
) = delete
```




<hr>



### function run 

_Run the node, blocking until SIGINT/SIGTERM or_ [_**stop()**_](classconduit_1_1Node.md#function-stop) _is called._
```C++
void conduit::Node::run () 
```



Starts all subscription threads and loop threads, installs signal handlers, and blocks the calling thread. 


        

<hr>



### function running 

_Check if the node is currently running._ 
```C++
bool conduit::Node::running () const
```





**Returns:**

true if [**run()**](classconduit_1_1Node.md#function-run) has been called and [**stop()**](classconduit_1_1Node.md#function-stop) has not yet completed. 





        

<hr>



### function stop 

_Stop the node (can be called from any thread or signal handler)._ 
```C++
void conduit::Node::stop () 
```




<hr>



### function ~Node 

```C++
virtual conduit::Node::~Node () 
```




<hr>
## Protected Functions Documentation




### function advertise 

_Create a typed publisher for the given topic._ 
```C++
template<typename T>
Publisher< T > conduit::Node::advertise (
    const std::string & topic,
    const PublisherOptions & options={}
) 
```





**Template parameters:**


* `T` [**Message**](structconduit_1_1Message.md) type to publish. 



**Parameters:**


* `topic` Topic name. 
* `options` Publisher configuration. 



**Returns:**

A Publisher&lt;T&gt; ready to publish messages. 





        

<hr>



### function loop [1/2]

_Register a fixed-rate loop with a member function callback._ 
```C++
template<typename T, typename Func>
void conduit::Node::loop (
    double rate_hz,
    Func T::* callback
) 
```





**Template parameters:**


* `T` Derived [**Node**](classconduit_1_1Node.md) type. 
* `Func` Member function pointer type. 



**Parameters:**


* `rate_hz` Loop frequency in Hz. 
* `callback` Member function to call each iteration. 




        

<hr>



### function loop [2/2]

_Register a fixed-rate loop with a lambda or std::function callback._ 
```C++
void conduit::Node::loop (
    double rate_hz,
    std::function< void()> callback
) 
```





**Parameters:**


* `rate_hz` Loop frequency in Hz. 
* `callback` Function to call each iteration. 




        

<hr>



### function subscribe [1/3]

_Subscribe to a topic with a member function callback (raw)._ 
```C++
template<typename T, typename Func>
void conduit::Node::subscribe (
    const std::string & topic,
    Func T::* callback
) 
```





**Template parameters:**


* `T` Derived [**Node**](classconduit_1_1Node.md) type. 
* `Func` Member function pointer type. 



**Parameters:**


* `topic` Topic name to subscribe to. 
* `callback` Member function to call on each message. 




        

<hr>



### function subscribe [2/3]

_Subscribe to a topic with a typed member function callback._ 
```C++
template<typename MsgT, typename T>
void conduit::Node::subscribe (
    const std::string & topic,
    void(T::*)(const TypedMessage< MsgT > &) callback
) 
```



Messages are automatically deserialized to MsgT before invoking the callback.




**Template parameters:**


* `MsgT` [**Message**](structconduit_1_1Message.md) type to deserialize into. 
* `T` Derived [**Node**](classconduit_1_1Node.md) type. 



**Parameters:**


* `topic` Topic name to subscribe to. 
* `callback` Member function receiving TypedMessage&lt;MsgT&gt;. 




        

<hr>



### function subscribe [3/3]

_Subscribe to a topic with a lambda or std::function callback (raw)._ 
```C++
void conduit::Node::subscribe (
    const std::string & topic,
    std::function< void(const Message &)> callback
) 
```





**Parameters:**


* `topic` Topic name to subscribe to. 
* `callback` Function invoked with each raw [**Message**](structconduit_1_1Message.md). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/node.hpp`

