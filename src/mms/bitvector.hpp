#pragma once
#include "../rtl/sdmconfig.h"

namespace sdm { namespace mms {
    
    template <typename element_t, std::size_t n_elements>
    struct bitvector {

      static constexpr std::size_t dimensions =  n_elements * sizeof(element_t) * CHAR_BITS;
      
      // TODO subscript operator with raw address arithmetic or something
      
      element_t& operator[](const unsigned i) {
        return _base[i];
      }
      
      /// XXX NEW XXX what's the use case that't not part of copy constructor?
      void copy_me(element_t* here) {
#pragma unroll
#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          here[i] = (*this)[i];
        }
      }
        
      /// SDM bitvector properties
        
      inline const std::size_t count() {
        std::size_t count = 0;
        for (unsigned i=0; i < n_elements; ++i) {
#if VELEMENT_64
          count += __builtin_popcountll(*(this[i]);
#else
          count += __builtin_popcount((*this)[i]);
#endif
        }
        return count;
      }
                
      inline const double density() {
        return (double) count() / dimensions;
      }
        
        
      // bitvector measurement functions
      
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
      
      /// Similarity of bitvectors
      inline double similarity(const bitvector& v) {
        // inverse of the normalized distance
        return 1.0 - (double) distance(v)/dimensions;
      }
      
      
      /// Overlap of bitvectors
      inline double overlap(const bitvector& v) {
        // ratio of common bits to max common
        return (double) inner(v)/dimensions;
      }
      
      
      ///////////////////////////////////////
      /// in place transactions on bitvectors //
      ///////////////////////////////////////
      
      
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
      
      
      /* add or superpose */
      
      inline void superpose(const bitvector& v) {
#pragma unroll
#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          (*this)[i] |= v[i];
        }
      }
      
      
      /* subtract v from u */
      
      inline void subtract(const bitvector& v) {
#pragma unroll
#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          (*this)[i] &= ~v[i];
        }
      }
      
      inline void multiply(const bitvector& v) {
#pragma unroll
#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          (*this)[i] ^= v[i];
        }
      }
      
      // bit twiddling utilities
      
      template<typename C>
      inline void whitebits(const C& v) {
        // half
        typename C::size_type h = v.size();
          // clear          
        for (auto it = v.begin(); it < v.begin() + h; ++it) {
          typename C::value_type r = *it; 
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          (*this)[i] &= ~(ONE << b);
        }
        // set
        for (auto it = v.begin() + h; it < v.end(); ++it) {
          typename C::value_type r = *it;
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          (*this)[i] |= (ONE << b);
        }
      }
      
      /* see above -- template these
      // set bits from elemental vector
      inline void whitebits(const basis_t& v) {
      unsigned h = v.size() / 2;
      // clear
      for (auto it = v.begin(); it < v.begin() + h; ++it) {
      unsigned r = *it;
      unsigned i = r / (sizeof(element_t) * CHAR_BITS);
      unsigned b = r % (sizeof(element_t) * CHAR_BITS);
      (*this)[i] &= ~(ONE << b); //XXX
      }
      // set
      for (auto it = v.begin() + h; it < v.end(); ++it) {
      unsigned r = *it;
      unsigned i = r / (sizeof(element_t) * CHAR_BITS);
      unsigned b = r % (sizeof(element_t) * CHAR_BITS);
      (*this)[i] |= (ONE << b);
      }
      }
      
      /*/
      
      // set bits from basic vector
      template<typename C>
      inline void setbits(const C& v) {
        for (auto r: v) {
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          (*this)[i] |= (ONE << b);
        }
      }

      
      // set from a vector of bit indexes
      inline void setbits(const std::vector<std::size_t>::iterator& start,
                          const std::vector<std::size_t>::iterator& end) {
        for (auto it = start; it < end; ++it){
          std::size_t r = *it;
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          (*this)[i] |= (ONE << b);
        }
      }

    private:
      element_t* _base;
      
    }; // end bitvector
  }}
