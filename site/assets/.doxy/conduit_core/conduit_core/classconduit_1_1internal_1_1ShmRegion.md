

# Class conduit::internal::ShmRegion



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md)



_RAII wrapper for a POSIX shared memory region._ [More...](#detailed-description)

* `#include <shm_region.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ShmRegion**](#function-shmregion-13) ([**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) && other) noexcept<br>_Move constructor._  |
|   | [**ShmRegion**](#function-shmregion-23) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) &) = delete<br> |
|  [**void**](classconduit_1_1internal_1_1Publisher.md) \* | [**data**](#function-data-12) () <br>_Get a writable pointer to the mapped memory._  |
|  [**const**](classconduit_1_1internal_1_1Publisher.md) [**void**](classconduit_1_1internal_1_1Publisher.md) \* | [**data**](#function-data-22) () const<br>_Get a read-only pointer to the mapped memory._  |
|  [**const**](classconduit_1_1internal_1_1Publisher.md) std::string & | [**name**](#function-name) () const<br>_Get the region name._  |
|  [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) & | [**operator=**](#function-operator) ([**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) && other) noexcept<br>_Move assignment operator._  |
|  [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) & | [**operator=**](#function-operator_1) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) &) = delete<br> |
|  [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**size**](#function-size) () const<br>_Get the size of the mapped region._  |
|   | [**~ShmRegion**](#function-shmregion) () <br>_Destructor. Unmaps the memory region (does_ **not** _unlink)._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) | [**create**](#function-create) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & name, [**size\_t**](classconduit_1_1internal_1_1Publisher.md) size) <br>_Create a new shared memory region (used by publishers)._  |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**exists**](#function-exists) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & name) <br>_Check if a shared memory region exists._  |
|  [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) | [**open**](#function-open) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & name) <br>_Open an existing shared memory region (used by subscribers)._  |
|  [**void**](classconduit_1_1internal_1_1Publisher.md) | [**unlink**](#function-unlink) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & name) <br>_Remove the shared memory file from the filesystem._  |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**wait\_until\_exists**](#function-wait_until_exists) ([**const**](classconduit_1_1internal_1_1Publisher.md) std::string & name, [**const**](classconduit_1_1internal_1_1Publisher.md) std::atomic&lt; [**bool**](classconduit_1_1internal_1_1Publisher.md) &gt; & running, std::chrono::milliseconds poll\_interval=std::chrono::milliseconds(100)) <br>_Poll until the shared memory region exists or running becomes false._  |


























## Detailed Description


Provides factory methods to create (publisher) or open (subscriber) a named shared memory region at `/dev/shm/conduit_{name}`. The region is memory-mapped on construction and unmapped on destruction, but the shared memory file is **not** unlinked automatically — call [**unlink()**](classconduit_1_1internal_1_1ShmRegion.md#function-unlink) explicitly when the topic is retired.




**See also:** [**Publisher**](classconduit_1_1internal_1_1Publisher.md), [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) 



    
## Public Functions Documentation




### function ShmRegion [1/3]

_Move constructor._ 
```C++
conduit::internal::ShmRegion::ShmRegion (
    ShmRegion && other
) noexcept
```




<hr>



### function ShmRegion [2/3]

```C++
conduit::internal::ShmRegion::ShmRegion (
    const  ShmRegion &
) = delete
```




<hr>



### function data [1/2]

_Get a writable pointer to the mapped memory._ 
```C++
inline void * conduit::internal::ShmRegion::data () 
```





**Returns:**

Pointer to the start of the region. 





        

<hr>



### function data [2/2]

_Get a read-only pointer to the mapped memory._ 
```C++
inline const  void * conduit::internal::ShmRegion::data () const
```





**Returns:**

Const pointer to the start of the region. 





        

<hr>



### function name 

_Get the region name._ 
```C++
inline const std::string & conduit::internal::ShmRegion::name () const
```





**Returns:**

Reference to the name string. 





        

<hr>



### function operator= 

_Move assignment operator._ 
```C++
ShmRegion & conduit::internal::ShmRegion::operator= (
    ShmRegion && other
) noexcept
```




<hr>



### function operator= 

```C++
ShmRegion & conduit::internal::ShmRegion::operator= (
    const  ShmRegion &
) = delete
```




<hr>



### function size 

_Get the size of the mapped region._ 
```C++
inline size_t conduit::internal::ShmRegion::size () const
```





**Returns:**

Size in bytes. 





        

<hr>



### function ~ShmRegion 

_Destructor. Unmaps the memory region (does_ **not** _unlink)._
```C++
conduit::internal::ShmRegion::~ShmRegion () 
```




<hr>
## Public Static Functions Documentation




### function create 

_Create a new shared memory region (used by publishers)._ 
```C++
static ShmRegion conduit::internal::ShmRegion::create (
    const std::string & name,
    size_t size
) 
```





**Parameters:**


* `name` Region name (becomes `/dev/shm/conduit_{name}`). 
* `size` Size in bytes. 



**Returns:**

A mapped [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md). 




**Exception:**


* [**ShmError**](classconduit_1_1ShmError.md) If shm\_open or mmap fails. 




        

<hr>



### function exists 

_Check if a shared memory region exists._ 
```C++
static bool conduit::internal::ShmRegion::exists (
    const std::string & name
) 
```





**Parameters:**


* `name` Region name. 



**Returns:**

true if the region exists. 





        

<hr>



### function open 

_Open an existing shared memory region (used by subscribers)._ 
```C++
static ShmRegion conduit::internal::ShmRegion::open (
    const std::string & name
) 
```





**Parameters:**


* `name` Region name. 



**Returns:**

A mapped [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md). 




**Exception:**


* [**ShmError**](classconduit_1_1ShmError.md) If the region does not exist or mmap fails. 




        

<hr>



### function unlink 

_Remove the shared memory file from the filesystem._ 
```C++
static void conduit::internal::ShmRegion::unlink (
    const std::string & name
) 
```





**Parameters:**


* `name` Region name. 




        

<hr>



### function wait\_until\_exists 

_Poll until the shared memory region exists or running becomes false._ 
```C++
static bool conduit::internal::ShmRegion::wait_until_exists (
    const std::string & name,
    const std::atomic< bool > & running,
    std::chrono::milliseconds poll_interval=std::chrono::milliseconds(100)
) 
```





**Parameters:**


* `name` Region name. 
* `running` Atomic flag checked each poll cycle; set to false to abort. 
* `poll_interval` Time between existence checks. 



**Returns:**

true if the region now exists, false if stopped early. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/shm_region.hpp`

