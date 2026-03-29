

# Struct conduit::Node::Subscription



[**ClassList**](annotated.md) **>** [**Subscription**](structconduit_1_1Node_1_1Subscription.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::function&lt; void(const [**Message**](structconduit_1_1Message.md) &)&gt; | [**callback**](#variable-callback)  <br> |
|  std::unique\_ptr&lt; [**internal::Subscriber**](classconduit_1_1internal_1_1Subscriber.md) &gt; | [**subscriber**](#variable-subscriber)  <br> |
|  std::thread | [**thread**](#variable-thread)  <br> |
|  std::string | [**topic**](#variable-topic)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
std::function<void(const Message&)> conduit::Node::Subscription::callback;
```




<hr>



### variable subscriber 

```C++
std::unique_ptr<internal::Subscriber> conduit::Node::Subscription::subscriber;
```




<hr>



### variable thread 

```C++
std::thread conduit::Node::Subscription::thread;
```




<hr>



### variable topic 

```C++
std::string conduit::Node::Subscription::topic;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/node.hpp`

