

# Class conduit::flow::Executor



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**flow**](namespaceconduit_1_1flow.md) **>** [**Executor**](classconduit_1_1flow_1_1Executor.md)



_Executes a flow configuration (startup and shutdown sequences)._ [More...](#detailed-description)

* `#include <executor.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Executor**](#function-executor-12) ([**ExecutorOptions**](structconduit_1_1flow_1_1ExecutorOptions.md) options={}) <br>_Construct an executor with the given options._  |
|   | [**Executor**](#function-executor-22) (const [**Executor**](classconduit_1_1flow_1_1Executor.md) &) = delete<br> |
|  [**Executor**](classconduit_1_1flow_1_1Executor.md) & | [**operator=**](#function-operator) (const [**Executor**](classconduit_1_1flow_1_1Executor.md) &) = delete<br> |
|  int | [**run**](#function-run) (const [**FlowConfig**](structconduit_1_1flow_1_1FlowConfig.md) & config) <br>_Run the flow (blocks until shutdown completes)._  |
|  bool | [**running**](#function-running) () const<br>_Check if the executor is currently running a flow._  |
|  void | [**shutdown**](#function-shutdown) () <br>_Request graceful shutdown of all running nodes._  |
|   | [**~Executor**](#function-executor) () <br> |




























## Detailed Description


Launches nodes as child processes, handles wait steps and parallel groups, and manages graceful shutdown on SIGINT/SIGTERM. 


    
## Public Functions Documentation




### function Executor [1/2]

_Construct an executor with the given options._ 
```C++
explicit conduit::flow::Executor::Executor (
    ExecutorOptions options={}
) 
```





**Parameters:**


* `options` Execution options. 




        

<hr>



### function Executor [2/2]

```C++
conduit::flow::Executor::Executor (
    const Executor &
) = delete
```




<hr>



### function operator= 

```C++
Executor & conduit::flow::Executor::operator= (
    const Executor &
) = delete
```




<hr>



### function run 

_Run the flow (blocks until shutdown completes)._ 
```C++
int conduit::flow::Executor::run (
    const FlowConfig & config
) 
```





**Parameters:**


* `config` The flow configuration to execute. 



**Returns:**

Exit code (0 on clean shutdown). 





        

<hr>



### function running 

_Check if the executor is currently running a flow._ 
```C++
bool conduit::flow::Executor::running () const
```





**Returns:**

true if [**run()**](classconduit_1_1flow_1_1Executor.md#function-run) is active. 





        

<hr>



### function shutdown 

_Request graceful shutdown of all running nodes._ 
```C++
void conduit::flow::Executor::shutdown () 
```




<hr>



### function ~Executor 

```C++
conduit::flow::Executor::~Executor () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_flow/include/conduit_flow/executor.hpp`

