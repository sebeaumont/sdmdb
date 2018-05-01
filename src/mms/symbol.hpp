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

      typedef semantic_vector<segment_manager_t, SDM_VECTOR_ELEMENT_TYPE, SDM_VECTOR_ELEMS> semantic_vector_t;

      // sparse stored (immutable) fingerprint
      typedef elemental_vector<segment_manager_t, unsigned> elemental_vector_t;

      // state 
      unsigned int _instance;
      shared_string_t _name;
  
    private:
      elemental_vector_t _basis;
      semantic_vector_t _vector;
    
    public:
  
      // constructor with fingerprint
      symbol(const char* s,
             const std::vector<unsigned>& f,
             const allocator_t& a)
        : _instance(0),
          _name(s, a),
          _basis(f, elemental_bits, a),
          _vector(a) {}
    
  
      inline const std::string name(void) const {
        return std::string(_name.begin(), _name.end());
      }
  
    
      typedef elemental_vector_t basis_vector_t;

      /*
      inline const basis_vector_t& basis(void) const {
        return _basis;
      }

      inline const semantic_vector_t& vector(void) const {
        return _vector;
      }
      */
      
      /// printer
      
      friend std::ostream& operator<<(std::ostream& os, const symbol& s) {
        os << s._name << ":" << s._instance;
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
                
      inline const double density() {
        return (double) count() / dimensions;
      }
        
        
      // semantic_vector measurement functions
      
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
    
      /// Similarity of symbols
      inline double similarity(const symbol& v) {
        // inverse of the normalized distance
        return 1.0 - (double) distance(v)/dimensions;
      }
    
    
      /// Overlap of symbols
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

      inline void superpose(const symbol& v, bool whiten=false) {
        if (whiten) {
          // clear 1/2
          unsigned h = v._basis.size() / 2;
          for (auto it = v._basis.begin(); it < v._basis.begin() + h; ++it) {
            unsigned r = (*it + v._instance) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] &= ~(ONE << b);
          }
          // set 1/2
          for (auto it = v._basis.begin() + h; it < v._basis.end(); ++it) {
            unsigned r = (*it + v._instance) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] |= (ONE << b);
          } 
          
        } else {
          // set all
          for (auto it = v._basis.begin(); it < v._basis.end(); ++it) {
            unsigned r = (*it + v._instance) % dimensions;
            unsigned i = r / (sizeof(element_t) * CHAR_BITS);
            unsigned b = r % (sizeof(element_t) * CHAR_BITS);
            _vector[i] |= (ONE << b);
          }
        }
      }

      // set bits from iteration of indexes N.B. not bounds checked!!
      
      template<typename C>
      inline void setbits(const C& v) {
        for (auto r: v) {
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          _vector[i] |= (ONE << b);
        }
      }
      
      
      // set from a vector of bit indexes
      inline void setbits(const std::vector<std::size_t>::iterator& start,
                          const std::vector<std::size_t>::iterator& end) {
        for (auto it = start; it < end; ++it){
          std::size_t r = *it;
          unsigned i = r / (sizeof(element_t) * CHAR_BITS);
          unsigned b = r % (sizeof(element_t) * CHAR_BITS);
          _vector[i] |= (ONE << b);
          }
      }
      
    };
    
  }
}
