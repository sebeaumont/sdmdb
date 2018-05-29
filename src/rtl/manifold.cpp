#include "manifold.hpp"

namespace sdm {

  
  /// the constructor
  
  manifold::manifold(const std::string& mmf, const std::size_t size) :
    heapimage(mmf),
    inisize(size),
    heap(mapfile(mmf, size)) {

    // pre-load space cache (and workaroud some weirdness)
    for (std::string spacename: get_named_spaces())
      ensure_space_by_name(spacename);
  }
  


  
  //////////////////////////////////////////////
  /// Read only operations on manifold spaces //
  //////////////////////////////////////////////
  
  ////////////////////
  /// named vectors // 
  ////////////////////
  
  
  /// vector density
  
  std::pair<const status_t, const double>
  manifold::density(const std::string& sn,
                    const std::string& vn) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp == nullptr) {
      return std::make_pair(ESPACE, 0);
    } else {
      // non_const as density() function cannot be marked const!
      auto v = sp->get_mutable_symbol_by_name(vn);
      if (v) return  std::make_pair(AOLD, v->density());
      else return std::make_pair(ESYMBOL, 0);
    }
  }
  
  
  /// find symbols by prefix
  
  std::pair<status_t, manifold::symbol_list>
  manifold::prefix_search(const std::string& sn,
                          const std::string& vp) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp) return std::make_pair(AOK, sp->search(vp));
    else {
      manifold::space::symbol_iterator a, b;
      return std::make_pair(ESPACE, std::make_pair(a, b));
    }
  }

    
  /// compute semantic similarity between symbols
  
  const std::pair<const status_t, const double>
  manifold::similarity(const std::string& tvs,
                       const std::string& tvn,
                       const std::string& svs,
                       const std::string& svn) noexcept {

    // all sspaces and symbols must exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return std::make_pair(ESPACE, 0);

    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn);
    if (!target_sym) return std::make_pair(ESYMBOL, 0);

    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return std::make_pair(ESPACE, 0);

    auto source_sym = source_sp->get_symbol_by_name(svn);
    if (!source_sym) return std::make_pair(ESYMBOL, 0);

    return std::make_pair(AOLD, target_sym->similarity(*source_sym));
  }


  /// compute semantic overlap between symbols -- we should have a
  
  const std::pair<const status_t, const double>
  manifold::overlap(const std::string& tvs,
                    const std::string& tvn,
                    const std::string& svs,
                    const std::string& svn) noexcept {

    // all sspaces and symbols must exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return std::make_pair(ESPACE, 0);
    
    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn);
    if (!target_sym) return std::make_pair(ESYMBOL, 0);

    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return std::make_pair(ESPACE, 0);

    auto source_sym = source_sp->get_symbol_by_name(svn);
    if (!source_sym) return std::make_pair(ESYMBOL, 0);
    // 
    return std::make_pair(AOLD, target_sym->overlap(*source_sym));
  }

  //////////////////////////////////////////
  /// under construction -- new topo scheme
  
  status_t manifold::get_topology(const std::string& space, std::size_t n, topology_t& topo) {
    // step 1 get the space 
    manifold::space* sp = get_space_by_name(space);
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

  
  /// return cardinality of a space
    
  std::pair<status_t, std::size_t>
  manifold::get_space_cardinality(const std::string& sn) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp) return std::make_pair(AOK, sp->entries());
    else return std::make_pair(ESPACE, 0);
  }

  // lookup a space by name

  /*
    XXX 
    TODO check this if this is fixed
    this has weird behaviour -- hangs or throws assersion errors
    so I'm doing a workaround and cache all spaces at rts start up via ensure space_by_name
    probably be quicker...
    
    std::pair<manifold::space*, std::size_t>
    manifold::get_space_by_name(const std::string& name) {
    return heap.find<space>(name.c_str());
    }
  */
  
  // this is meant to be fast so no optional's here
  // XX might change i/f to return a status pair

  /// lookup all spaces in the heap/segment manager
  
  std::vector<std::string>
  manifold::get_named_spaces() noexcept {

    std::vector<std::string> names;
    
    typedef segment_t::const_named_iterator const_named_it;
    const_named_it named_beg = heap.named_begin();
    const_named_it named_end = heap.named_end();
    
    for(; named_beg != named_end; ++named_beg){
      const segment_t::char_type *name = named_beg->name();
      std::size_t name_len = named_beg->name_length();
      if (name[0] != '_')
        names.push_back(std::string(name, name_len));
      // constant void pointer to the named object
      // const void *value = named_beg->value();
    }
    return names;
  }

  
  // create and manage named symbols by name -- space constructor does find_or_construct on segment
  // then database memoizes pointers to spaces to speed up symbol resolution
  
  inline std::pair<status_t, manifold::space*>
  manifold::ensure_space_by_name(const std::string& name) {

    // lookup in cache
    auto it = spaces.find(name);
    
    if (it == spaces.end()) {
      // not found
      // delegate find_or_construct to symbol_space...
      // and create a runtime cache entry
      // XXX N.B. this coould fail if we run out of space
      try {
        space* sp = new space(name, heap);
        spaces[name] = sp;
        return std::make_pair(ANEW, sp);
        
      } catch (boost::interprocess::bad_alloc& e) {
        // try and expand memory and retry  or... 
        return std::make_pair(EMEMORY, nullptr);
      }
      
    } else {
      // used cached space
      return std::make_pair(AOLD, it->second);
    }
  }
  
}
