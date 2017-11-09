//
//  fast_random.h
//
//  Copyright (c) 2012-2016 Simon Beaumont - All rights reserved.
//

#pragma once

#include <limits>
#include <vector>
#include <algorithm>
#include <boost/random/random_device.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


namespace molemind { namespace sdm {

  namespace random {

    // this is current PRNG
    // UC conform to boost::
    
    struct xorshifter : private boost::noncopyable {
    
    private:

      uint64_t x;              
      uint64_t s[16];          
      int p = 0;

      
      // fastest PRNG  
      inline uint64_t xorshift64star(void) {
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        return x * UINT64_C(2685821657736338717);
      }

      // high quality PRNG with large period 2^1024 - 1
      inline uint64_t xorshift1024star(void) {
        uint64_t s0 = s[p];
        uint64_t s1 = s[p = (p+1) & 15];
        s1 ^= s1 << 31; // a
        s1 ^= s1 >> 11; // b
        s0 ^= s0 >> 30; // c
        return ( s[p] = s0 ^ s1 ) * UINT64_C(1181783497276652981);
      }

    public:
      
      // state must be seeded with a non-zero value
      explicit xorshifter(uint64_t seed) : x(seed) {
        // use xorshift64star to seed 16 64bits of state for
        // xosrshift1024start
      }

      inline uint64_t rand(void) {
        return xorshift1024star();
      }

      // conform to boost rng i/face
      typedef uint64_t result_type;

      static result_type min() { return std::numeric_limits<result_type>::max(); }
      static result_type max() { return std::numeric_limits<result_type>::max(); }

      result_type operator()() { return xorshift1024star(); }
    };


    // todo put our rng in here
      
    struct uniform_random : std::unary_function<unsigned, unsigned> {
     
      boost::random::mt19937& _rng;
      
      unsigned operator()(unsigned i) {
        boost::random::uniform_int_distribution<> roll(0, i-1);
        return roll(_rng);
      }
     
      explicit uniform_random(boost::random::mt19937& g) : _rng(g) {}
      
    };


    // shuffle based vector randomization tools
   
    class index_randomizer {
      
    private:
      
      boost::random::mt19937 _state;
      uniform_random _generator;
      //random_cmrrsr _cmrrsr; //= random_cmrrsr(428394849);
      //my_adapter _generator; //= my_adapter(_cmrrsr);
      std::vector<size_t> _idx; // why not size_t?

    public:
     
      index_randomizer(std::size_t n) : _state(boost::random::mt19937()),
                                        _generator(uniform_random(_state)) {
        _idx.reserve(n);
        // initialize list of indexes one time
        for (std::size_t i = 0; i < n; ++i)
          _idx.push_back(i);
      }
      
      
      // generate a shuffled vector of indexes could fix/template length of this
      
      inline std::vector<std::size_t>& shuffle(void) {
        std::random_shuffle(_idx.begin(), _idx.end(), _generator);
        return _idx;
      }
      
    };
    
  }
  }}
