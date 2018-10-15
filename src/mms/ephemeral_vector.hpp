#pragma once
#include "sdmconfig.h"
#include "bitvector.hpp"


namespace sdm {
  namespace mms {

    ///
    /// ephemeral_vector can wrap another vector and overload binary ops
    ///
    template <typename element_t,
              unsigned n_elements,
              typename W,
              class base_t=std::vector<element_t>>

    
    struct ephemeral_vector :public base_t {

      static constexpr std::size_t dimensions =  n_elements * sizeof(element_t) * CHAR_BITS;
      
      /// constructors
      ephemeral_vector() : base_t() {
        this->reserve(n_elements);
      }

      /// construct from API input type 
      explicit ephemeral_vector(const sdm_vector_t& other) : base_t() {
        this->reserve(n_elements);
        #pragma unroll
        for (unsigned i=0; i < n_elements; ++i)
          this->push_back(other[i]);
      }

      /////////////////////////////////////////////
      /// binary operations on wrapped vector type
      /////////////////////////////////////////////

      /// semantic distance is the Hamming distance
      
      inline const std::size_t distance(const W& v) const {
        std::size_t distance = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = (*this)[i] ^ v[i];
          #ifdef VELEMENT_64
          distance += __builtin_popcountll(r);
          #else
          distance += __builtin_popcount(r);
          #endif
        }
        return distance;
      }
    
      /// inner product is the commonality/overlap
      
      inline const std::size_t inner(const W& v) const {
        std::size_t count = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = (*this)[i] & v[i];
          #ifdef VELEMENT_64
          count += __builtin_popcountll(r);
          #else
          count += __builtin_popcount(r);
          #endif
        }
        return count;
      }
      
      /// semantic union
      
      inline const std::size_t countsum(const W& v) const {
        std::size_t count = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = (*this)[i] | v[i];
          #ifdef VELEMENT_64
          count += __builtin_popcountll(r);
          #else
          count += __builtin_popcount(r);
          #endif
        }
        return count;
      }
    
      /// semantic similarity
      
      inline const double similarity(W& v) const {
        // inverse of the normalized distance
        return 1.0 - (double) distance(v)/dimensions;
      }
    
    
      /// semantic overlap
      
      inline const double overlap(const W& v) const {
        // ratio of common bits to max common
        return (double) inner(v)/dimensions;
      }
    
    };
  }
}
