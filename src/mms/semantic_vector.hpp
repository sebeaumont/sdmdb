#pragma once

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include "../rtl/sdmconfig.h"

namespace sdm {
  namespace mms {

    namespace bip = boost::interprocess;

    ///
    /// semantic_vector represents state in managed memory space
    ///
    template <typename segment_manager_t, typename element_t, unsigned n_elements>    
    struct semantic_vector : public bip::vector<element_t, bip::allocator<element_t, segment_manager_t>> {

      typedef typename bip::allocator<void, segment_manager_t> void_allocator_t;
      
      typedef bip::vector<element_t, bip::allocator<element_t, segment_manager_t>> vector_base_t;
            
      /// construct fully
      semantic_vector(const void_allocator_t& a) : vector_base_t(a) {
        this->reserve(n_elements);
        #pragma unroll
        //#pragma clang loop vectorize(enable) interleave(enable)
        for (unsigned i = 0; i < n_elements; ++i) this->push_back(0);
      }

    };
  }
}
