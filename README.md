# PAR: The Parameterized Action Representation
Decription: [PAR](http://dl.acm.org/citation.cfm?id=371552.371567) is an action representation that can be used in conjunction with an agent archutecture. It contains methods to query a knowledge base and realize action commands written as hierarchical task networks. PAR written in C++ and MySQL. It connects to python as its scripting language.
## Requirements: [Python 2.7](https://www.python.org/), [MySQL with C++ connector](https://www.mysql.com/)
## Installation Quick Guide
This quick start guide will explain how to compile PAR in visual studio release mode for either 32-bit or
64-bit use. The architecture must stay consistent throughout (so if you plan on using 32bit, please consider
that when downloading all pre-requisite software). If you wish to use PAR in visual studio debug mode,
please refer to the much more in-depth installation instructions included in the WIKI
1. Download and install the MySQL server. Create a default connection on localhost with the username
root and password root. Create and Environmental Variable called MYSQL ROOT which points
to the root directory of this server.
1. Run the script actionary.sql included with PAR. This sets up basic tables and provides a default action
and object hierarchy. More information about the object hierarchy can be found in Object Hierarchy.
1. Download and install MySQL Connector C++. Create an Environmental Variable called CONNEC-
TOR ROOT which points to the root directory of this library. There is also a dll that is installed
with the connector (mysqlcppconn.dll ). Add the directory this dll is located in to the path.
1. Download and install the Python Programming language. Make sure to choose to add Python to
the path. Also create an environmental variable called PYTHON ROOT which points to the root
directory of your python installation (default installation is C:
Python27.
1. Run the script Compile64QuickStart.bat for 64bit or CompileQuickStart.bat for 32bit, included with
PAR. If everything is correct, three libraries should be made (lwnet.lib, database.lib, and agentProc.lib
in the libs directory).
## USAGE
Please see the wiki examples for techniques on using PAR.
## Credits
TBW
