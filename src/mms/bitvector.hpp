#pragma once
#include "../rtl/sdmconfig.h"

namespace sdm {
  namespace mms {

    /// fixed length or at least pre-allocated vectors using std::vector
    
    template <typename element_t,
              std::size_t n_elements,
              class base_t=std::vector<element_t>,
              class allocator=std::allocator<element_t>>
    
    struct bitvector
      :public base_t {

      static constexpr std::size_t dimensions =  n_elements * sizeof(element_t) * CHAR_BITS;
      
      ///////////////////
      /// constructors //
      ///////////////////
      
      bitvector() : base_t() {
        this->reserve(n_elements);
      }
      
      explicit bitvector(const allocator& a) : base_t(a) {
        this->reserve(n_elements);
      }


      ////////////////////////////////
      /// SDM bitvector arithmetic ///
      ////////////////////////////////
      
      inline const std::size_t count() {
        std::size_t count = 0;
        for (unsigned i=0; i < n_elements; ++i) {
          #if VELEMENT_64
          count += __builtin_popcountll((*this)[i]);
          #else
          count += __builtin_popcount((*this)[i]);
          #endif
        }  
        return count;
      }

      /// semantic density
      
      inline const double density() {
        return (double) count() / dimensions;
      }
        
      /////////////////////////////////////////// 
      // semantic_vector measurement functions //
      ///////////////////////////////////////////

      /// semantic distance is the Hamming distance
      
      inline const std::size_t distance(const bitvector& v) {
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
      
      inline const std::size_t inner(const bitvector& v) {
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
      
      inline std::size_t countsum(const bitvector& v) {
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
      
      inline double similarity(const bitvector& v) {
        // inverse of the normalized distance
        return 1.0 - (double) distance(v)/dimensions;
      }
    
    
      /// semantic overlap
      
      inline double overlap(const bitvector& v) {
        // ratio of common bits to max common
        return (double) inner(v)/dimensions;
      }
    
    
      /////////////////////////////////////////
      /// in place transactions onbitvectors //
      /////////////////////////////////////////
    
    
      /* set all bits */
    
      inline void ones(void) {
        for (unsigned i=0; i < n_elements; ++i) {
          (*this)[i] = -1;
        }
      }
    
      /* clear all bits */
    
      inline void zeros(void) {
        for (unsigned i=0; i < n_elements; ++i) {
          (*this)[i] = 0;
        }
      }

    };
    
  }
}
