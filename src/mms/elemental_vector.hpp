// Copyright (c) 2016 Simon Beaumont - All Rights Reserved.

#pragma once

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>

namespace molemind {
  
  namespace sdm {

    namespace mms {

      namespace bip = boost::interprocess;
      
      template <typename T, typename M>
      struct elemental_vector : public bip::vector<T, bip::allocator<T, M>> {
        
        typedef bip::vector<T, bip::allocator<T, M>> vector_t;
        typedef bip::allocator<bip::vector<T, bip::allocator<T, M>>, M> bitv_vector_allocator_t;
        typedef typename bip::allocator<void, M> void_allocator_t;
        
        //elemental_vector(std::size_t s, const void_allocator_t& a) : vector_t(s, 0, a) {}
        
        elemental_vector(const std::vector<std::size_t>& fs, const:: size_t s, const void_allocator_t& a) : vector_t(fs.begin(), fs.begin()+s, a) {}

      };
    }
  }
}
