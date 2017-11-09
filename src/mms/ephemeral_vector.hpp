// Copyright (c) 2017 Simon Beaumont - All Rights Reserved.

#pragma once

#define VELEMENT_64 1

#ifdef VELEMENT_64
#define ONE 1ULL
#else
#define ONE 1U
#endif
#define CHAR_BITS (8)


namespace molemind {
  
  namespace sdm {

    namespace mms {

      // what to call this really now we have it?
      
      template <typename T, std::size_t L, typename I>
      struct ephemeral_vector {
        
        ephemeral_vector() {
        }

        ephemeral_vector(const T* v) {
          // XXX input not bounds checked
          #pragma unroll
          #pragma clang loop vectorize(enable) interleave(enable)
          for (std::size_t i=0; i < L; ++i, ++v) {
            _elem[i] = *v;
          }
        }
  
        
        ephemeral_vector(const I& v) {
          for (std::size_t i = 0; i < L; ++i) {
            _elem[i] = v[i];
          }
        }

        T operator[](const std::size_t i) {
          return _elem[i];
        }
        
        /// SDM dimensions i.e. bits
        static constexpr std::size_t dimensions =  L * sizeof(T) * CHAR_BITS;
        

        /// SDM vector properties
        
        inline const std::size_t count() {
          std::size_t count = 0;
          for (std::size_t i=0; i < L; ++i) {
            #if VELEMENT_64
            count += __builtin_popcountll(_elem[i]);
            #else
            count += __builtin_popcount((_elem[i]);
            #endif
          }
          return count;
        }
        
        
        inline double density() {
          return (double) count() / dimensions;
        }
        
        
        // vector measurement functions
        
        inline const std::size_t distance(ephemeral_vector& v) {
          std::size_t distance = 0;
          #pragma unroll
          #pragma clang loop vectorize(enable) interleave(enable)
          for (std::size_t i=0; i < L; ++i) {
            T r = _elem[i] ^ v._elem[i];
            #ifdef VELEMENT_64
            distance += __builtin_popcountll(r);
            #else
            distance += __builtin_popcount(r);
            #endif
          }
          return distance;
        }

        
        inline const std::size_t distance(const I& v) const {
          std::size_t distance = 0;
          #pragma unroll
          #pragma clang loop vectorize(enable) interleave(enable)
          for (std::size_t i=0; i < L; ++i) {
            T r = _elem[i] ^ v[i];
            #ifdef VELEMENT_64
            distance += __builtin_popcountll(r);
            #else
            distance += __builtin_popcount(r);
            #endif
          }
          return distance;
        }
        
        
        inline const std::size_t inner(ephemeral_vector& v) {
          std::size_t count = 0;
          #pragma unroll
          #pragma clang loop vectorize(enable) interleave(enable)
          for (std::size_t i=0; i < L; ++i) {
            T r = _elem[i] & v._elem[i];
            #ifdef VELEMENT_64
            count += __builtin_popcountll(r);
            #else
            count += __builtin_popcount(r);
            #endif
          }
          return count;
        }
        
        
        inline std::size_t countsum(ephemeral_vector& v) {
          std::size_t count = 0;
          #pragma unroll
          #pragma clang loop vectorize(enable) interleave(enable)
          for (std::size_t i=0; i < L; ++i) {
            T r = _elem[i] | v._elem[i];
            #ifdef VELEMENT_64
            count += __builtin_popcountll(r);
            #else
            count += __builtin_popcount(r);
            #endif
          }
          return count;
        }
        
                                        
        /// Similarity of vectors
        inline double similarity(ephemeral_vector& v) {
          // inverse of the normalized distance
          return 1.0 - (double) distance(v)/dimensions;
        }
                                        
        /// Similarity of vectors
        inline double similarity(const I& v) const {
          // inverse of the normalized distance
          return 1.0 - (double) distance(v)/dimensions;
        }
                                      
                                        

      private:
        T _elem[L];
      };
    }
  }
}
