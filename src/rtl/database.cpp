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
    if (check_heap_sanity()) heap.flush();
  }
  
  
  
  ////////////////////
  /// named vectors // 
  ////////////////////
  
  
  // fully guarded operations e.g.
  
  /// vector density
  
  boost::optional<const double>
  database::density(const std::string& sn,
                    const std::string& vn) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp == nullptr) {
      return boost::none;
    } else {
      // non_const as density() function cannot be marked const!
      auto v = sp->get_mutable_symbol_by_name(vn);
      if (v) return  v->density(); else return boost::none;
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

  // XXX TODO look at inlining this so we can re-use
  
  status_t
  database::ensure_symbol(const std::string& sn,
                          const std::string& vn) noexcept {
    // may create a space
    auto space = ensure_space_by_name(sn);
    if (!space) return ERUNTIME;
    
    auto sym = space->get_symbol_by_name(vn);
    if (sym) {
      return AOLD; // found
      
    } else try {
        // use insert to create a new symbol with elemental "fingerprint"
        sym = space->insert_mutable_symbol(vn, irand.shuffle());
        
        if (sym) {
          // insert successful:
          // N.B. the returned symbol reference is to an *immutable* entry in the index
          // i.e. const database::space::symbol& s = *(p.first);
          return ANEW; // created
          
        } else return EINDEX; // something in the index stopped us inserting!
        
      } catch (boost::interprocess::bad_alloc& e) {
        // XXX: here is where we can try and grow the heap
        return EMEMORY; // 'cos we ran out of memory!
      }
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
  
  status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::string& sn,
                      const bool newbasis) noexcept {
    
    // assume all symbols are present
    status_t state = AOLD;
    
    auto target_sp = ensure_space_by_name(ts);
    if (!target_sp) return ESPACE;
    
    auto source_sp = ensure_space_by_name(ss);
    if (!source_sp) return ESPACE;
    
    
    // get source symbol and optionally generate a new version or basis
    boost::optional<const space::symbol&> s = source_sp->get_symbol_by_name(sn, newbasis);

    if (!s) {
      // try inserting source symbol
      s = source_sp->insert_symbol(sn, irand.shuffle());
      if (!s) return EINDEX;
      state = ANEW; // => a symbol was created possbily within a new space.
    }
    
    // get target symbol
    
    //////////////////////////////////////////////////////////////////////
    // CAVEAT: this must follow any insertions in the space
    // as any insert to index MAY invalidate vector or symbol pointers...
    
    boost::optional<space::symbol&> t = target_sp->get_mutable_symbol_by_name(tn);
    
    if (!t) {
      // try inserting the target symbol
      t = target_sp->insert_mutable_symbol(tn, irand.shuffle());
      if (!t) return EINDEX; // something stopped us inserting inspite of not being found!
      state = ANEW;          // a symbol wss created possbily within a new space.
    }

    // do the update to the target symbol
    t->superpose(*s);
    return state;
  }


  /// batch superpose target with multiple symbols from source space
  status_t
  database::superpose(const std::string& ts,
                      const std::string& tn,
                      const std::string& ss,
                      const std::vector<std::string>& sns,
                      const bool newbasis) noexcept {
    // XXX TODO
    return EUNIMPLEMENTED;
  }
    
  /// remove source from target -- source and target must exist else this is a noop.
  
  status_t
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
    return AOK;
  }

  
  /// compute semantic similarity between symbols
  
  boost::optional<double>
  database::similarity(const std::string& tvs,
                       const std::string& tvn,
                       const std::string& svs,
                       const std::string& svn) noexcept {

    // all sspaces and symbols must exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return ESPACE;

    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn);
    if (!target_sym) return ESYMBOL;

    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return ESPACE;

    auto source_sym = source_sp->get_symbol_by_name(svn);
    if (!source_sym) return ESYMBOL;

    // 
    return target_sym->similarity(*source_sym);
  }

  /// compute semantic overlap between symbols
  
  boost::optional<double>
  database::overlap(const std::string& tvs,
                    const std::string& tvn,
                    const std::string& svs,
                    const std::string& svn) noexcept {

    // all sspaces and symbols must exist
    auto target_sp = get_space_by_name(tvs);
    if (!target_sp) return ESPACE;
    
    auto target_sym = target_sp->get_mutable_symbol_by_name(tvn);
    if (!target_sym) return ESYMBOL;

    auto source_sp = get_space_by_name(svs);
    if (!source_sp) return ESPACE;

    auto source_sym = source_sp->get_symbol_by_name(svn);
    if (!source_sym) return ESYMBOL;
    // 
    return target_sym->overlap(*source_sym);
  }

  
  //////////////////////
  // space management //
  //////////////////////
  
  // create and manage named symbols by name -- space constructor does find_or_construct on segment
  // then database memoizes pointers to spaces to speed up symbol resolution
  
  database::space* database::ensure_space_by_name(const std::string& name) {
    // lookup in cache
    auto it = spaces.find(name);
    
    if (it == spaces.end()) {
      // delegate find_or_construct to symbol_space...
      // and create a runtime cache entry
      space* sp = new space(name, heap);
      spaces[name] = sp;
      return sp;
      
    } else {
      // used cached space
      return it->second;
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
  
  // this is meant to be fast so no optional's here -- we could inline this.
  database::space* database::get_space_by_name(const std::string& name) {
    auto it = spaces.find(name);
    if (it == spaces.end())
      return nullptr;
    else
      return it->second;
    }
  
  // destroy space permanently
  
  bool database::destroy_space(const std::string& name) noexcept {
    return heap.destroy<space>(name.c_str());
  }
  
  // lookup all spaces in the heap/segment manager
  
  std::vector<std::string> database::get_named_spaces() noexcept {
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
      //const void *value = named_beg->value();
    }
    return names;
  }
    
    
  boost::optional<std::size_t> database::get_space_cardinality(const std::string& sn) noexcept {
    auto sp = get_space_by_name(sn);
    if (sp) return sp->entries();
    else return boost::none;
  }
  
  ////////////////////////
  // gc, heap management
  ////////////////////////
    
  bool database::grow_heap_by(const std::size_t& extra_bytes) noexcept {
    // mapped_file grow
    // todo unmap heap
    //
    if (heap.grow(heapimage.c_str(), extra_bytes)) {
      // remap
      heap = segment_t(bip::open_only, heapimage.c_str());
      std::cout << "free: " << free_heap() << " max: " << maxheap << " init:" << inisize << " heap:" << heap_size() << std::endl;
      if (check_heap_sanity()) return true;
    }
    return false;
  }
  
  bool database::compactify_heap() noexcept {
      // mapped_file shrink_to_fit -- compact
    return heap.shrink_to_fit(heapimage.c_str());
  }
  
}


