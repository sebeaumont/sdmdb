SDMLIB - Sparse Distributed Memory Database 
===========================================

Unreleased Researchware

The main artefact of the project is the sdm runtime library which
offers a high performance sparse distributed memory or semantic vector
database targeted at a number of platforms.

The sdm library is currently written in C++ making extensive use of
boost multiprocessing template library to provide memory management
tools to allow the supporting data structures to be allocated from a
memory mapped file, thus providing a database and RTL/API to act on it.

The application is high performance associative memory to further
research and development of the capabilities of a general model of
hyper-dimensional sparse space. This model has been extensively
applied to and indeed was incubated in the domain of data/text
mining particularly word level embeddings.

The architecture of this library is a bit of a moving target but I am
settling on providing a minimal core library as a header only
implementation and small c/c++ modules to facilitate integration into
various foreign language environments. Watch this space. 


Building 
-------- 

Requires cmake and (recommended) out of source build the build type
Release is cruicial (unless a default Debug build is required) else
performance will be an order of magnitude or more worse.

```shell
mkdir build;
cd build;
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
make test && make install
```

C++ Documentation
-----------------

Requires doxygen

```shell
doxygen


Project contact: [Simon Beaumont](mailto:datalligator@icloud.com) 
_______________________
Copyright (c) 2012-2018 Simon Beaumont.

See: LICENSE for terms and conditions under which this software is
made available.


