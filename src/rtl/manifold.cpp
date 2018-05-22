#include "manifold.hpp"

namespace sdm {

  /// inherit explicit database constructor
  manifold::manifold(const std::size_t size,
                     const std::size_t max,
                     const std::string& image,
                     const bool compact)
    : database::database(size, max, image, compact) {}

  
  status_t manifold::get_topology(const std::string& space, topology_t& topo) {
    // step 1 get the space 
    database::space* sp = get_space_by_name(space);
    if (!sp) return ESPACE; // space not found
    std::size_t card = sp->entries();
    // allocate a topo which is just the vectors in the space
    topo.reserve(card);
    // now copy the symbols_spaces vectors from the space into caller
    for (std::size_t i = 0; i < card; ++i) {
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
