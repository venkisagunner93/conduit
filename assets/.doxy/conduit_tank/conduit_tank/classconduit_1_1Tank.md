

# Class conduit::Tank



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Tank**](classconduit_1_1Tank.md)



_MCAP-based message recorder with Zstd/LZ4 compression._ [More...](#detailed-description)

* `#include <tank.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Tank**](#function-tank-13) (const std::string & output\_path) <br>_Construct a recorder targeting the given output file._  |
|   | [**Tank**](#function-tank-23) (const [**Tank**](classconduit_1_1Tank.md) &) = delete<br> |
|   | [**Tank**](#function-tank-33) ([**Tank**](classconduit_1_1Tank.md) &&) = delete<br> |
|  void | [**add\_topic**](#function-add_topic) (const std::string & topic) <br>_Add a topic to record (must be called before_ [_**start()**_](classconduit_1_1Tank.md#function-start) _)._ |
|  uint64\_t | [**message\_count**](#function-message_count) () const<br>_Get the total number of messages recorded so far._  |
|  [**Tank**](classconduit_1_1Tank.md) & | [**operator=**](#function-operator) (const [**Tank**](classconduit_1_1Tank.md) &) = delete<br> |
|  [**Tank**](classconduit_1_1Tank.md) & | [**operator=**](#function-operator_1) ([**Tank**](classconduit_1_1Tank.md) &&) = delete<br> |
|  bool | [**recording**](#function-recording) () const<br>_Check if the recorder is currently active._  |
|  void | [**start**](#function-start) () <br>_Start recording messages from all added topics._  |
|  void | [**stop**](#function-stop) () <br>_Stop recording and finalize the MCAP file._  |
|   | [**~Tank**](#function-tank) () <br> |




























## Detailed Description


Records messages from one or more topics into an MCAP file. Topics must be added before calling [**start()**](classconduit_1_1Tank.md#function-start). The recorded file can be replayed with the conduit CLI tools.




**See also:** Node 



    
## Public Functions Documentation




### function Tank [1/3]

_Construct a recorder targeting the given output file._ 
```C++
explicit conduit::Tank::Tank (
    const std::string & output_path
) 
```





**Parameters:**


* `output_path` Path to the MCAP output file. 




        

<hr>



### function Tank [2/3]

```C++
conduit::Tank::Tank (
    const Tank &
) = delete
```




<hr>



### function Tank [3/3]

```C++
conduit::Tank::Tank (
    Tank &&
) = delete
```




<hr>



### function add\_topic 

_Add a topic to record (must be called before_ [_**start()**_](classconduit_1_1Tank.md#function-start) _)._
```C++
void conduit::Tank::add_topic (
    const std::string & topic
) 
```





**Parameters:**


* `topic` Topic name to subscribe to and record. 




        

<hr>



### function message\_count 

_Get the total number of messages recorded so far._ 
```C++
uint64_t conduit::Tank::message_count () const
```





**Returns:**

Message count. 





        

<hr>



### function operator= 

```C++
Tank & conduit::Tank::operator= (
    const Tank &
) = delete
```




<hr>



### function operator= 

```C++
Tank & conduit::Tank::operator= (
    Tank &&
) = delete
```




<hr>



### function recording 

_Check if the recorder is currently active._ 
```C++
bool conduit::Tank::recording () const
```





**Returns:**

true if recording is in progress. 





        

<hr>



### function start 

_Start recording messages from all added topics._ 
```C++
void conduit::Tank::start () 
```




<hr>



### function stop 

_Stop recording and finalize the MCAP file._ 
```C++
void conduit::Tank::stop () 
```




<hr>



### function ~Tank 

```C++
conduit::Tank::~Tank () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_tank/include/conduit_tank/tank.hpp`

