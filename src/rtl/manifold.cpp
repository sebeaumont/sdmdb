#include "manifold.hpp"

namespace sdm {

  /// inherit explicit database constructor
  manifold::manifold(const std::size_t size,
                     const std::size_t max,
                     const std::string& image,
                     const bool compact)
    : database::database(size, max, image, compact) {}

  
  status_t manifold::get_topology(const std::string& space, std::size_t n, topology_t& topo) {
    // step 1 get the space 
    database::space* sp = get_space_by_name(space);
    if (!sp) return ESPACE; // space not found

    // just in case the caller asked for more than there is or indeed
    // the world changed in the meantime.
    
    std::size_t card = sp->entries();
    // allocate a topo which is just the vectors in the space
    //topo.reserve(card);
    // now copy the symbols_spaces vectors from the space into caller
    for (std::size_t i = 0; i < card && i < n; ++i) {
      auto s = sp->symbol_at(i);
      auto v = s.vector();
      vector_t b;
      // manual copy of vector yetch...
      #pragma unroll
      for (std::size_t j = 0; j < SDM_VECTOR_ELEMS; ++j)  b[j] = v[j];
      topo.push_back(b);
    }
    return AOK;
  }
  
}
