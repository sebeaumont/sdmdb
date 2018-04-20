# Runtime template library and test harness

## State of art

1. Only symbol_space is now implemented -- this now allocates all
   memory for elemental and sematic vectors and indexes thus is the most expensive.
   It currenly takes about 5 us to fully instantiate and index a 16 K
   vector. Randomizing the elemental part might be the main
   overhead.
   
2. Offer superposition option to use saturated (setbits) vs. dithered
   (whitebits) learning.
   
3. Seperate learning from search and hold semantic vectors in the
   symbol rather than in sequential array -- this facilitates deletion
   or selective search (runtime) cache in cpu or gpu and saves on
   database pre-allocation. 
   
   What does this imply for space separation and architecture? 
   Do all symbols need to have elemental and semantic vectors?
   
4. Leverage above search engine facilities in DSL. Hybrid!



## Core TODO

1. Bit vector initialisation including randomization of elemental
   white basis vectors (EWBV). Check this before release.

2. Semantic/Elemental/Composite vector algebra at space level should
   implement at binary_vector level. Not sure what this means now.

3. Extending/shrinking mmf heap - this is a global runtime concern. 

4. C Wrapper and C++ library fascade
   build as shared library, (bitcode?) re-implement test
   harness/command line tool. This is more or less done for FFI purposes.

5. Benchmarks

## iOS Port 

1. Watch this space. Actually this has been successuly done at some
   stage and had a Swift interface library. Wait 'til API is settled down.
