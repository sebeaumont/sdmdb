#pragma once

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>

#include "sdmconfig.h"
#include "bitvector.hpp"


namespace sdm {
  namespace mms {

    namespace bip = boost::interprocess;

    ///
    /// semantic_vector represents state in managed memory space
    ///
    template <typename segment_manager_t,
              typename element_t,
              unsigned n_elements>

    
    struct semantic_vector
      : public bitvector<element_t,
                         n_elements,
                         bip::vector<element_t, bip::allocator<element_t, segment_manager_t>>,
                         bip::allocator<void, segment_manager_t>> {
      //: public bip::vector<element_t, bip::allocator<element_t, segment_manager_t>> {

      typedef typename bip::allocator<void, segment_manager_t> void_allocator_t;
      
      // typedef bip::vector<element_t, bip::allocator<element_t, segment_manager_t>> vector_base_t;

      typedef bitvector<element_t,
                        n_elements,
                        bip::vector<element_t, bip::allocator<element_t, segment_manager_t>>,
                        bip::allocator<void, segment_manager_t>> vector_base_t;

      
      /// construct fully
      semantic_vector(const void_allocator_t& a) : vector_base_t(a) {
        this->reserve(n_elements);
        #pragma unroll
        //#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i = 0; i < n_elements; ++i) this->push_back(0);
      }

      /// copy vector to aligned destination with no bounds checks
      // TODO: benchmark this against bounded and unrolled loop
      inline void copyto(element_t* dst) {
        std::memcpy(dst, this->data(), n_elements * sizeof(element_t));
      }
      
    };
  }
}
