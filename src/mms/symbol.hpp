#pragma once
#include "semantic_vector.hpp"
#include "elemental_vector.hpp"
#include "../rtl/sdmconfig.h"

namespace sdm {

  namespace mms {
  
    ///
    /// symbol - named vector with lazily computed elemental fingerprint
    ///

    template <typename segment_manager_t, typename shared_string_t, typename allocator_t>
  
    struct symbol final {
      
      typedef SDM_VECTOR_ELEMENT_TYPE element_t;

      static constexpr unsigned n_elements = SDM_VECTOR_ELEMS;
      
      static constexpr unsigned dimensions =  n_elements * sizeof(element_t) * CHAR_BITS;

      // elemental bits

      static constexpr unsigned elemental_bits = SDM_VECTOR_BASIS_SIZE;    

      // stored semantic bit vector

      typedef semantic_vector<segment_manager_t,
                              SDM_VECTOR_ELEMENT_TYPE,
                              SDM_VECTOR_ELEMS> semantic_vector_t;

      // sparse stored (immutable) fingerprint

      typedef elemental_vector<segment_manager_t, unsigned> elemental_vector_t;

      // state
      enum type { normal, white, pink, brown };

      shared_string_t _name;
      unsigned _refcount;
      type _type;
      
    private:
      
      elemental_vector_t _basis;
      semantic_vector_t _vector;
      
    public:
  
      /// symbol constructor with immuatable fingerprint
      
      symbol(const char* s,
             const std::vector<unsigned>& f,
             const allocator_t& a,
             const type t = normal)
        : _name(s, a),
          _refcount(0),
          _type(t),
          _basis(f, elemental_bits, a),
          _vector(a) {}
    
      /// copy of name (XXX why? seems unecessary) try unit tests without this!

      inline const std::string name(void) const {
        return std::string(_name.begin(), _name.end());
      }
  
    
      /// printer for symbol XXX might be useful to dump symbol representation to stream 
      
      friend std::ostream& operator<<(std::ostream& os, const symbol& s) {
        os << s._name << "`" << s._refcount;
        return os;
      }

      /// SDM bitvector/semantic_vector delegated properties
        
      inline const std::size_t count() {
        std::size_t count = 0;
        for (unsigned i=0; i < n_elements; ++i) {
          #if VELEMENT_64
          count += __builtin_popcountll(_vector[i]);
          #else
          count += __builtin_popcount(_vector[i]);
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
      
      inline const std::size_t distance(const symbol& v) {
        std::size_t distance = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = _vector[i] ^ v._vector[i];
          #ifdef VELEMENT_64
          distance += __builtin_popcountll(r);
          #else
          distance += __builtin_popcount(r);
          #endif
        }
        return distance;
      }
    
      /// inner product is the commonality/overlap
      
      inline const std::size_t inner(const symbol& v) {
        std::size_t count = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = _vector[i] & v._vector[i];
          #ifdef VELEMENT_64
          count += __builtin_popcountll(r);
          #else
          count += __builtin_popcount(r);
          #endif
        }
        return count;
      }
      
      /// semantic union
      
      inline std::size_t countsum(const symbol& v) {
        std::size_t count = 0;
        #pragma unroll
        #pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i=0; i < n_elements; ++i) {
          element_t r = _vector[i] | v._vector[i];
          #ifdef VELEMENT_64
          count += __builtin_popcountll(r);
          #else
          count += __builtin_popcount(r);
          #endif
        }
        return count;
      }
    
      /// semantic similarity
      
      inline double similarity(const symbol& v) {
        // inverse of the normalized distance
        return 1.0 - (double) distance(v)/dimensions;
      }
    
    
      /// semantic overlap
      
      inline double overlap(const symbol& v) {
        // ratio of common bits to max common
        return (double) inner(v)/dimensions;
      }
    
    
      ////////////////////////////////////////////////
      /// in place transactions on semantic vectors //
      ////////////////////////////////////////////////
    
    
      /* set all bits */
    
      inline void ones(void) {
        for (unsigned i=0; i < n_elements; ++i) {
          _vector[i] = -1;
        }
      }
    
      /* clear all bits */
    
      inline void zeros(void) {
        for (unsigned i=0; i < n_elements; ++i) {
          _vector[i] = 0;
        }
      }

      /////////////////////////
      /// learning utilities //
      /////////////////////////

      inline void superpose(const symbol& v, int rotations = 0) {
        if (v._type == white) {
          // clear 1/2
          unsigned h = v._basis.size() / 2;
          for (auto it = v._basis.begin(); it < v._basis.begin() + h; ++it) {
            unsigned r = (*it + rotations) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] &= ~(ONE << b);
          }
          // set 1/2
          for (auto it = v._basis.begin() + h; it < v._basis.end(); ++it) {
            unsigned r = (*it + rotations) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] |= (ONE << b);
          } 
          
        } else {
          // set all
          for (auto it = v._basis.begin(); it < v._basis.end(); ++it) {
            unsigned r = (*it + rotations) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] |= (ONE << b);
          }
        }
      }

      ///////////////////////////////////////////////////////////////////////////
      // remove elemental bits of all instances 
      // XXX TODO we could select a set of instance indexes and default to this
      
      inline void subtract(const symbol&v, int rotations=0) {
        // we must subtract rotated basis of source vector

        if (v._type == white) {
          unsigned h = v._basis.size() / 2;
          // clear top 1/2
          for (auto it = v._basis.begin() + h; it < v._basis.end(); ++it) {
            unsigned r = (*it + rotations) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] &= ~(ONE << b);
          }
          
        } else {
          // clear all instances
          for (auto it = v._basis.begin(); it < v._basis.end(); ++it) {
            unsigned r = (*it + rotations) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] &= ~(ONE << b);
          }
        }
      }
      
    };
    
  }
}
