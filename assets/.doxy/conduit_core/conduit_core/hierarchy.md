
# Class Hierarchy

This inheritance list is sorted roughly, but not completely, alphabetically:


* **class** [**conduit::Node**](classconduit_1_1Node.md) _Base class for conduit processing nodes._ 
* **class** [**conduit::internal::Publisher**](classconduit_1_1internal_1_1Publisher.md) _Low-level publisher that writes raw bytes to a shared memory ring buffer._ 
* **class** [**conduit::internal::RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md) _Reader side of the lock-free SPMC ring buffer._ 
* **class** [**conduit::internal::RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md) _Writer side of the lock-free SPMC ring buffer._ 
* **class** [**conduit::internal::ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) _RAII wrapper for a POSIX shared memory region._ 
* **class** [**conduit::internal::Subscriber**](classconduit_1_1internal_1_1Subscriber.md) _Low-level subscriber that reads raw bytes from a shared memory ring buffer._ 
* **class** [**conduit::Publisher**](classconduit_1_1Publisher.md) _Type-safe publisher that serializes messages of type T._ 
* **class** [**conduit::Subscriber**](classconduit_1_1Subscriber.md) _Type-safe subscriber that deserializes messages of type T._ 
* **struct** [**conduit::Message**](structconduit_1_1Message.md) _Raw message received from a topic._ 
* **struct** [**conduit::PublisherOptions**](structconduit_1_1PublisherOptions.md) _Configuration for topic publisher and ring buffer sizing._ 
* **struct** [**conduit::SubscriberOptions**](structconduit_1_1SubscriberOptions.md) _Configuration for topic subscriber._ 
* **struct** [**conduit::internal::AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md) _Cache-line-aligned atomic uint64\_t to prevent false sharing._ 
* **struct** [**conduit::internal::ReadResult**](structconduit_1_1internal_1_1ReadResult.md) _Result of a successful ring buffer read._ 
* **struct** [**conduit::internal::RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) _Ring buffer configuration._ 
* **struct** [**conduit::internal::RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) _Shared memory layout for the ring buffer control structure._ 
* **struct** [**conduit::Node::Loop**](structconduit_1_1Node_1_1Loop.md) 
* **struct** [**conduit::Node::Subscription**](structconduit_1_1Node_1_1Subscription.md) 
* **struct** [**conduit::TypedMessage**](structconduit_1_1TypedMessage.md) _Typed message with deserialized payload._ 
* **class** **std::runtime_error**    
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **class** [**conduit::ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._     
        * **class** [**conduit::NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
        * **class** [**conduit::PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
        * **class** [**conduit::RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
        * **class** [**conduit::ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
        * **class** [**conduit::SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
        * **class** [**conduit::TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 

