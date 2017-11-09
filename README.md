SDMLIB - Sparse Distributed Memory Database 
===========================================

The main artefact is the sdm runtime library which offers a high
performance sparse distributed memory or semantic vector database
targeted at a number of platforms.

The sdm library is currently written in C++ making extensive use of
boost multiprocessing template library to provide memory management
tools to allow the supporting data structures to be allocated from a
memory mapped file, thus providing a database and RTL/API to act on it.

The application is high performance associative memory to further
research and development of the capabilities of a general model of
hyper-dimensional sparse space. This model has been extensively
applied to and indeed was incubated in the domain of data/text
mining.

There is a c library under active development currently driven
by the need for a foreign function interface for client languages.


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

Will generate doc -- N.B. a C++ library is not currently built for
standalone C++ clients as the entire library is more or less template
based. This is unlikely to change unless this thing proves to be of
use outside beyond scope of the research it now supports. 

The c library is a very cut down shim of the full capability of the
C++ implementation and serves the ad-hoc needs of the FFI. 



Project contact: [Simon Beaumont](mailto:s@molemind.net) 
_______________________
Copyright (c) 2012-2018 Simon Beaumont.

See: LICENSE for terms and conditions under which this software is
made available.


