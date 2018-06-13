// Copyright (c) 2015, 2016 Simon Beaumont - All Rights Reserved

/// Implementation of sdm::database
//#include "manifold.hpp"
#include "database.hpp"

/* TODO rationalise and make consistent this API!!! */

namespace sdm {
  
  /// constructor to initialize database

  database::database(const std::string& mmf,
                     const std::size_t initial_size,
                     const std::size_t max_size,
                     const bool compact)
    
    // N.B. Constructor does not inherit from manifold implmentation as we open or create
    // the heap r/w

    : manifold(mmf, initial_size),
      maxheap(max_size),          // maximum size of heap in bytes
      compclose(compact),         // compact heap on close?
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
  
  
  /////////////////////////////////////////////////////////
  /// get an existing (or create new) symbol.
  ///
  /// N.B. can cause memory outage and may side-effect
  /// the creation of a space and a symbol+vector within it

  // XXX TODO look at inlining a version of this so we can re-use
  
  const sdm_status_t
  database::namedvector(const std::string& sn,
                        const std::string& vn,
                        const sdm_prob_t p) noexcept {

    auto symp = ensure_symbol(sn, vn, p);
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
  
  const sdm_status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::string& sn,
                      const int shifted,
                      const bool refcount) noexcept {
    
    // assume all symbols are present
    sdm_status_t state = AOLD;
    
    auto tsp = ensure_space_by_name(ts);
    if (sdm_error(tsp.first)) return tsp.first;
    
    auto ssp = ensure_space_by_name(ss);
    if (sdm_error(ssp.first)) return ssp.first;
    
    
    // get source symbol
    // TODO: optionally generate a new version or shifted basis
    // and otionally reference count 
    //boost::optional<const space::symbol&> s = ssp.second->get_symbol_by_name(sn, refcount);
    boost::optional<space::symbol&> s = ssp.second->get_mutable_symbol_by_name(sn, refcount);

    if (!s) {
      // try inserting source symbol
      //s = ssp.second->insert_symbol(sn, irand.shuffle());
      s = ssp.second->insert_mutable_symbol(sn, irand.shuffle());
      if (!s) return EINDEX;
      state = ANEW; // => a symbol was created possbily within a new space.
    }
    
    // get target symbol
    
    //////////////////////////////////////////////////////////////////////
    // CAVEAT: this must follow any insertions in the space
    // as any insert to index MAY invalidate vector or symbol pointers...
    
    boost::optional<space::symbol&> t = tsp.second->get_mutable_symbol_by_name(tn, false);
    
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

  const sdm_status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::vector<const std::string>& sns,
                      const std::vector<const int> shifts,
                      const bool refcount) noexcept {
    
    // XXX new approach to keep allocations to minimum places in code
    //sdm_status_t sts = ensure_mutable_symbol
    return EUNIMPLEMENTED;
  }
    
  /// remove source from target -- source and target must exist else this is a noop.
  
  const sdm_status_t
  database::subtract(const std::string& tvs, 
                     const std::string& tvn,
                     const std::string& svs,
                     const std::string& svn) noexcept {

    // N.B. all spaces and symbols should exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return ESPACE;

    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn, false);
    if (!target_sym) return ESYMBOL;
    
    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return ESPACE;

    auto source_sym = source_sp->get_symbol_by_name(svn, false); // arguable decref ;-)
    if (!source_sym) return ESYMBOL;

    // effect
    target_sym->subtract(*source_sym);
    return AOLD;
  }

  //////////////////////
  // space management //
  //////////////////////
  
  /// destroy space permanently
  
  bool
  database::destroy_space(const std::string& name) noexcept {
    return heap.destroy<space>(name.c_str());
  }
  
  
  // XXX inline allocators refactoring 
  
  //////////////////////////////////////////
  /// inline private or protected utilities
  //////////////////////////////////////////
  /*
  inline std::pair<sdm_status_t, space::symbol&>
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

  inline std::pair<sdm_status_t, const database::space::symbol*>
  database::ensure_symbol(const std::string& spacename,
                          const std::string& name,
                          const sdm_prob_t dither,
                          const bool refcount) {

    auto sp = ensure_space_by_name(spacename);
    if (sdm_error(sp.first)) return std::make_pair(sp.first, nullptr);
    
    auto s = sp.second->get_symbol_by_name(name, refcount);
    
    if (!s) {
      // try inserting new symbol
      try {
        s = sp.second->insert_symbol(name, irand.shuffle(), dither);
        if (s) return std::make_pair(ANEW, &(*s));
        else return std::make_pair(EINDEX, nullptr);

      } catch (boost::interprocess::bad_alloc& e) {
        // try and expand memory and retry or... 
        return std::make_pair(EMEMORY, nullptr);
      }
    }
    else return std::make_pair(AOLD, &(*s));
  }
  
  

  /////////////////////
  /// heap management
  /////////////////////za
    
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


