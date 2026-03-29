
# Class List


Here are the classes, structs, unions and interfaces with brief descriptions:

* **namespace** [**conduit**](namespaceconduit.md)     
    * **class** [**ConduitError**](classconduit_1_1ConduitError.md) _Base exception for all conduit errors._ 
    * **struct** [**Message**](structconduit_1_1Message.md) _Raw message received from a topic._     
    * **class** [**Node**](classconduit_1_1Node.md) _Base class for conduit processing nodes._     
    * **class** [**NodeError**](classconduit_1_1NodeError.md) _Error in_ [_**Node**_](classconduit_1_1Node.md) _lifecycle (run, stop, signal handling)._
    * **class** [**PublisherError**](classconduit_1_1PublisherError.md) _Error during publisher creation or publishing._ 
    * **struct** [**PublisherOptions**](structconduit_1_1PublisherOptions.md) _Configuration for topic publisher and ring buffer sizing._     
    * **class** [**RingBufferError**](classconduit_1_1RingBufferError.md) _Error in ring buffer operations (invalid config, overwrite detected)._ 
    * **class** [**ShmError**](classconduit_1_1ShmError.md) _Error during shared memory operations (create, open, map)._ 
    * **class** [**SubscriberError**](classconduit_1_1SubscriberError.md) _Error during subscriber creation or message reading._ 
    * **struct** [**SubscriberOptions**](structconduit_1_1SubscriberOptions.md) _Configuration for topic subscriber._ 
    * **class** [**TankError**](classconduit_1_1TankError.md) _Error during MCAP recording (file I/O, codec failures)._ 
    * **namespace** [**internal**](namespaceconduit_1_1internal.md) _Internal implementation details for conduit._     
        * **struct** [**AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md) _Cache-line-aligned atomic uint64\_t to prevent false sharing._     
        * **class** [**Publisher**](classconduit_1_1internal_1_1Publisher.md) _Low-level publisher that writes raw bytes to a shared memory ring buffer._     
        * **struct** [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) _Result of a successful ring buffer read._     
        * **struct** [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) _Ring buffer configuration._     
        * **struct** [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) _Shared memory layout for the ring buffer control structure._     
        * **class** [**RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md) _Reader side of the lock-free SPMC ring buffer._     
        * **class** [**RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md) _Writer side of the lock-free SPMC ring buffer._     
        * **class** [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) _RAII wrapper for a POSIX shared memory region._     
        * **class** [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) _Low-level subscriber that reads raw bytes from a shared memory ring buffer._     
    * **namespace** [**log**](namespaceconduit_1_1log.md) _Conduit logging utilities._     
* **struct** [**Loop**](structconduit_1_1Node_1_1Loop.md)     
* **struct** [**Subscription**](structconduit_1_1Node_1_1Subscription.md)     
* **class** [**Publisher**](classconduit_1_1Publisher.md) _Type-safe publisher that serializes messages of type T._     
* **class** [**Subscriber**](classconduit_1_1Subscriber.md) _Type-safe subscriber that deserializes messages of type T._     
* **struct** [**TypedMessage**](structconduit_1_1TypedMessage.md) _Typed message with deserialized payload._     
* **namespace** [**std**](namespacestd.md) 

