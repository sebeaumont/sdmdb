// Copyright (c) 2015, 2016 Simon Beaumont - All Rights Reserved

/// Implementation of sdm::database

#include "database.hpp"

/* TODO rationalise and make consistent this API!!! */

namespace sdm {
  
  /// constructor to initialize database

  database::database(const std::size_t initial_size,
                     const std::size_t max_size,
                     const std::string& mmf,
                     const bool compact)
    // init slots
    : inisize(initial_size),      // initial size of heap in bytes
      maxheap(max_size),          // maximum size of heap in bytes
      // construct the memory mapped segment for database
      heap(bip::open_or_create, mmf.c_str(), initial_size),
      heapimage(mmf),             // diskimage path
      compclose(compact),           // compact heap on close?
      // initialize PRNG
      irand(random::index_randomizer(space::symbol::dimensions)) {
    
    // pre-load space cache (and workaroud some weirdness)
    for (std::string spacename: get_named_spaces())
      ensure_space_by_name(spacename);
  }
    
    
  /// destructor flushes the segment iff sane
  
  database::~database() {
    if (check_heap_sanity()) {
      heap.flush();
      if (compclose) compactify_heap();
    }
  }
  
  
  
  ////////////////////
  /// named vectors // 
  ////////////////////
  
  
  // fully guarded operations e.g.
  
  /// vector density
  
  std::pair<const status_t, const double>
  database::density(const std::string& sn,
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
  
  boost::optional<database::symbol_list>
  database::prefix_search(const std::string& sn,
                          const std::string& vp) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp) return sp->search(vp);
    else return boost::none;
  }
  
  
  /////////////////////////////////////////////////////////
  /// get an existing (or create new) symbol.
  ///
  /// N.B. can cause memory outage and may side-effect
  /// the creation of a space and a symbol+vector within it

  // XXX TODO look at inlining a version of this so we can re-use
  
  const status_t
  database::namedvector(const std::string& sn,
                        const std::string& vn,
                        const space::symbol::type ty) noexcept {

    auto symp = ensure_symbol(sn, vn, ty);
    return symp.first;
  }


  //////////////////////////////////////
  /// learning/transactional operations
  //////////////////////////////////////
  
  ////////////////////////////////////////
  /// superpose target vector with source
  ///
  
  // N.B. may side effect creation of spaces and symbols as a convenience
  // for realtime training and thus cause memory outage which may incur
  // database growth if possbile (or indeed may fail) TODO exception handling
  // and attempt growth on bad_alloc
  
  const status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::string& sn,
                      const int shifted) noexcept {
    
    // assume all symbols are present
    status_t state = AOLD;
    
    auto tsp = ensure_space_by_name(ts);
    if (sdm_error(tsp.first)) return tsp.first;
    
    auto ssp = ensure_space_by_name(ss);
    if (sdm_error(ssp.first)) return ssp.first;
    
    
    // get source symbol and optionally generate a new version or basis
    boost::optional<const space::symbol&> s = ssp.second->get_symbol_by_name(sn);

    if (!s) {
      // try inserting source symbol
      s = ssp.second->insert_symbol(sn, irand.shuffle());
      if (!s) return EINDEX;
      state = ANEW; // => a symbol was created possbily within a new space.
    }
    
    // get target symbol
    
    //////////////////////////////////////////////////////////////////////
    // CAVEAT: this must follow any insertions in the space
    // as any insert to index MAY invalidate vector or symbol pointers...
    
    boost::optional<space::symbol&> t = tsp.second->get_mutable_symbol_by_name(tn);
    
    if (!t) {
      // try inserting the target symbol
      t = tsp.second->insert_mutable_symbol(tn, irand.shuffle());
      if (!t) return EINDEX; // something stopped us inserting inspite of not being found!
      state = ANEW;          // a symbol wss created possbily within a new space.
    }

    // do the update to the target symbol
    t->superpose(*s);
    return state;
  }


  /// batch superpose target with multiple symbols from source space

  const status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::vector<const std::string>& sns,
                      const std::vector<const int> shifts) noexcept {
    
    // XXX new approach to keep allocations to minimum places in code
    //status_t sts = ensure_mutable_symbol
    return EUNIMPLEMENTED;
  }
    
  /// remove source from target -- source and target must exist else this is a noop.
  
  const status_t
  database::subtract(const std::string& tvs, 
                     const std::string& tvn,
                     const std::string& svs,
                     const std::string& svn) noexcept {

    // N.B. all spaces and symbols should exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return ESPACE;

    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn);
    if (!target_sym) return ESYMBOL;
    
    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return ESPACE;

    auto source_sym = source_sp->get_symbol_by_name(svn);
    if (!source_sym) return ESYMBOL;

    // effect
    target_sym->subtract(*source_sym);
    return AOLD;
  }

  
  /// compute semantic similarity between symbols
  
  const std::pair<const status_t, const double>
  database::similarity(const std::string& tvs,
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
  database::overlap(const std::string& tvs,
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

  
  //////////////////////
  // space management //
  //////////////////////
  
  /// destroy space permanently
  
  bool
  database::destroy_space(const std::string& name) noexcept {
    return heap.destroy<space>(name.c_str());
  }
  
  /// lookup all spaces in the heap/segment manager
  
  std::vector<std::string>
  database::get_named_spaces() noexcept {

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

  /// return cardinality of a space
    
  boost::optional<std::size_t>
  database::get_space_cardinality(const std::string& sn) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp) return sp->entries();
    else return boost::none;
  }
  
  // XXX inline allocators refactoring 
  
  //////////////////////////////////////////
  /// inline private or protected utilities
  //////////////////////////////////////////
  /*
  inline std::pair<status_t, space::symbol&>
  database::ensure_mutable_symbol(const std::string& spacename,
                                  const std::string& name,
                                  const space::symbol::type type) {
    
    space* sp = ensure_space_by_name(spacename);
    
    auto s = sp->get_mutable_symbol_by_name(name);
    // if not found try and insert new symbol
    if (!s)
      return sp->insert_mutable_symbol(name, irand.shuffle(), type) ? ANEW : EINDEX;
    else
      return AOLD;
  }
  */

  inline std::pair<status_t, const database::space::symbol*>
  database::ensure_symbol(const std::string& spacename,
                          const std::string& name,
                          const space::symbol::type type) {

    auto sp = ensure_space_by_name(spacename);
    if (sdm_error(sp.first)) return std::make_pair(sp.first, nullptr);
    
    auto s = sp.second->get_symbol_by_name(name);
    
    if (!s) {
      // try inserting new symbol
      try {
        s = sp.second->insert_symbol(name, irand.shuffle(), type);
        if (s) return std::make_pair(ANEW, &(*s));
        else return std::make_pair(EINDEX, nullptr);

      } catch (boost::interprocess::bad_alloc& e) {
        // try and expand memory and retry or... 
        return std::make_pair(EMEMORY, nullptr);
      }
    }
    else return std::make_pair(AOLD, &(*s));
  }
  
  
  // create and manage named symbols by name -- space constructor does find_or_construct on segment
  // then database memoizes pointers to spaces to speed up symbol resolution
  
  inline std::pair<status_t, database::space*>
  database::ensure_space_by_name(const std::string& name) {

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
  
  // lookup a space by name

  /*
    XXX 
    TODO check this if this is fixed
    this has weird behaviour -- hangs or throws assersion errors
    so I'm doing a workaround and cache all spaces at rts start up via ensure space_by_name
    probably be quicker...
    
    std::pair<database::space*, std::size_t>
    database::get_space_by_name(const std::string& name) {
    return heap.find<space>(name.c_str());
    }
  */
  
  // this is meant to be fast so no optional's here
  inline database::space*
  database::get_space_by_name(const std::string& name) noexcept {
    auto it = spaces.find(name);
    if (it == spaces.end())
      return nullptr;
    else
      return it->second;
  }

   ////////////////////////
  // gc, heap management
  ////////////////////////
    
  bool
  database::grow_heap_by(const std::size_t& extra_bytes) noexcept {
    // XXX totally untested
    isexpanding = true;
    
    if (heap.grow(heapimage.c_str(), extra_bytes)) {
      // remap... shoud we unmap first?
      heap = segment_t(bip::open_only, heapimage.c_str());
      isexpanding = false;
      // 
      std::cout << "free: " << free_heap()
                << " max: " << maxheap
                << " init:" << inisize
                << " heap:" << heap_size()
                << std::endl;
      
      if (check_heap_sanity()) return true;
    }
    return false;
  }
  
  bool database::compactify_heap() noexcept {
    // mapped_file shrink_to_fit -- compact
    return heap.shrink_to_fit(heapimage.c_str());
  }
  
}


