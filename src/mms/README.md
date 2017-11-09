# Runtime template library and test harness

## State of art

1. Only feature_space is fully implemented -- this now allocates all
   memory for elemental and sematic vectors and indexes thus is the most expensive.
   It currenly takes about 5 us to fully instantiate and index a 16 K
   trit vector. Randomizing the elemental part might be the main
   overhead.
   

## Core TODO

1. Bit vector initialisation including randomization of elemental
   white basis vectors (EWBV).

2. Semantic/Elemental/Composite vector algebra at space level should
   implement at binary_vector level.

3. Extending/shrinking mmf heap - this is a global runtime concern. 

4. C Wrapper and C++ library fascade
   build as shared library, (bitcode?) re-implement test
   harness/command line tool.

5. Benchmarks

## iOS Port 

1. Watch this space.
