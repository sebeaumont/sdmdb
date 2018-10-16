SDMLIB - Sparse Distributed Memory Database 
===========================================

Unreleased Apha version -- Planned Public Release Milestone December 2018

I am currently refactoring the architecture of the API and library
into two parts, the database and the query engine. This is to allow a
more compact and flexible training/learning implementation and to use
GPU or dedicated hardware implmentation for the metric space/topology
or search egnine.

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

There is a c library under active development currently driven
by the need for a foreign function interface for client languages.
This currently supercedes the c++ library classes and may end up as
the only API. Tho' is seems a shame to miss out on integrations using
say boost python or other clever and efficient FFI bindings.



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
```

The c library is a very cut down shim of the full capability of the
C++ implementation and serves the ad-hoc needs of the FFI indeed this
may likely be the minimal specification that is required and all the
other methods in the C++ API may be elided in due course. Indeed we
may decline to even build a C++ library in future.


Project contact: [Simon Beaumont](mailto:s@molemind.net) 
_______________________
Copyright (c) 2012-2018 Simon Beaumont.

See: LICENSE for terms and conditions under which this software is
made available.


