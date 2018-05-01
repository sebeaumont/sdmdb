// Copyright (c) 2012-2018 Simon Beaumont - All Rights Reserved.

#pragma once

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>

namespace sdm {
  
  namespace mms {
    
    namespace bip = boost::interprocess;
    
    template <typename segment_manager_t, typename index_t>
    
    // sparse vector 
    struct elemental_vector : public bip::vector<index_t, bip::allocator<index_t, segment_manager_t>> {
      
      typedef bip::vector<index_t, bip::allocator<index_t, segment_manager_t>> container_t;

      typedef typename bip::allocator<void, segment_manager_t> void_allocator_t;
      
      // constructor with initialiser
      elemental_vector(const std::vector<unsigned>& fs,
                       const unsigned s,
                       const void_allocator_t& a) : container_t(fs.begin(), fs.begin()+s, a) {}
      
    };
  }
}

