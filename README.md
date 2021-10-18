# PAR: The Parameterized Action Representation
Decription: [PAR](http://dl.acm.org/citation.cfm?id=371552.371567) is an action representation that can be used in conjunction with an agent architecture. It contains methods to query a knowledge base and realize action commands written as hierarchical task networks. PAR written in C++ and MySQL. It connects to python as its scripting language. There have been several changes to PAR over the years. The most recent documentation can currently be found in my dissertation.
## Requirements: [Python 3.X](https://www.python.org/), [SQLITE DLL](https://www.sqlite.org/download.html)
## Installation Quick Guide
This quick start guide will explain how to compile PAR in visual studio release mode for either 32-bit or
64-bit use. The architecture must stay consistent throughout (so if you plan on using 32bit, please consider
that when downloading all pre-requisite software). If you wish to use PAR in visual studio debug mode,
please refer to the much more in-depth installation instructions included in the WIKI
1. Download and install the SQLITE DLL and Source Code from the download page. The source should be saved into a folder called include. The dll should be saved to a folder libs.
Create an environmental variable named SQLITE_ROOT that points to the root directory that holds both the include and libs file.
1. The default database comes with everything in actionary.sql. This sets up basic tables and provides a default action
and object hierarchy. More information about the object hierarchy can be found in Object Hierarchy.
1. Download and install the Python Programming language. Make sure to choose to add Python to
the path. Also create an environmental variable called PYTHON_ROOT which points to the root
directory of your python installation (default installation is C:\Python3X.)
1. Run the script Compile64QuickStart.bat for 64bit or CompileQuickStart.bat for 32bit, included with
PAR. If everything is correct, three libraries should be made (lwnet.lib, database.lib, and agentProc.lib
in the libs directory). Note: If this does not create the libraries, you will have to open each of them and compile them manually.
## USAGE
Please see the wiki examples for techniques on using PAR.
## Credits
TBW
