[![CodeFactor](https://www.codefactor.io/repository/github/r00tdroid/datawise2/badge)](https://www.codefactor.io/repository/github/r00tdroid/datawise2)

# DataWise2
DataWise 2 is a software package that provides analytics tool for your projects. It has native integration for Unity and Unreal Engine 4.

The toolset consists of the following module:
## Core
Shared library that provides the public API to interact with the rest of the toolset

## Server
Server-side application that manages database access and data compilation

## DWC
Commandline interface to interact with the server

## Test
A test application to generate data and upload it to the server

# Building
The main components of the toolset are build with [Scons](https://scons.org)

1. Open a commandprompt on the `Toolset` directory
2. Execute the `scons --module [module name]` command to build the specified module
