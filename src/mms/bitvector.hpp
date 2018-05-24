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
      

      // constructors
      bitvector() : base_t() {
        this->reserve(n_elements);
      }
      
      explicit bitvector(const allocator& a) : base_t(a) {
        this->reserve(n_elements);
      }

      

    
      // TODO: borrow entire implementation from symbol
    };
    
  }
}
