# Digitizers and AWGs Communication API
___

__1. Operating Environment__

This project is used to provide an efficient communication interface for self-developed digitizers and AWGs. The project relies on 
[WinpCap](https://www.winpcap.org/). You should ensure that your computer has WinPcap installed before calling the DLL.

__2. Compile Step__

We use the VS2010 environment to compile the project. The steps are as follows.

* 1 Create solution: Create a blank solution and add the source files. 
* 2 Configure the solution: __[Solution]__->__[Properties]__->__[Regular]__->__[Configure Type]__->__[.dll]__
* 3 Add additional dependencies for ADC_COM_API: __iphlpapi.lib__, __ws2_32.lib__
* 4 Compile and build the project.


