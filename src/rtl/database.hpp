// Copyright (c) 2015 Simon Beaumont - All Rights Reserved
// runtime.hpp - runtime api interface

#pragma once
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/optional.hpp>
#include <map>


#include "../mms/symbol_space.hpp"
#include "../util/fast_random.hpp"

//#include "topology.hpp"
#include "sdmconfig.h"
#include "sdmstatus.h"

#include <iostream>

namespace sdm {

  namespace bip = boost::interprocess;
  
  /***********************************************************************
   ** Database type provides the API for the SDM implementation
   ***********************************************************************/
  
  class database {

    /// memory manager for spaces within the database
    typedef bip::managed_mapped_file segment_t;
    
  public:

    /////////////////////////////////
    /// mms symbol_space definition
    /////////////////////////////////

    /// type of space implementation determines the type and number of elements and sparsity
    
    typedef mms::symbol_space<SDM_VECTOR_ELEMENT_TYPE,
                              SDM_VECTOR_ELEMS,
                              SDM_VECTOR_BASIS_SIZE,
                              segment_t> space;
    
    
    /// constructor to initialize file mapped heap
    
    explicit database(const std::size_t initial_size,
                      const std::size_t max_size,
                      const std::string& filepath,
                      const bool compact=false);

    
    /// no copy or move semantics

    database() = delete;
    database(const database&) = delete;
    database(database&&) = delete;
    const database& operator=(const database&) = delete;
    const database& operator=(database&&) = delete;

    /// destructor will cautiously ensure all pages are flushed
    
    ~database();

    /// search for symbols starting with prefix
    typedef std::pair<database::space::symbol_iterator, database::space::symbol_iterator> symbol_list;
    
    boost::optional<symbol_list>
    prefix_search(const std::string& space_name,
                  const std::string& symbol_prefix) noexcept;
    
    
    /////////////////////////
    /// vector properties ///
    /////////////////////////
    
    /// get vector density
    boost::optional<const double>
    density(const std::string& space_name,
            const std::string& vector_name) noexcept;


    /////////////////////////////////////////////////////
    /// effectful learning operations on target vectors
    /////////////////////////////////////////////////////

    
    /// lookup or create new symbol
    
    status_t
    ensure_symbol(const std::string& space_name,
                  const std::string& symbol_name) noexcept;

    
    /// add or superpose
    status_t
    superpose(const std::string& ts, const std::string& tn,
              const std::string& ss, const std::string& sn,
              const bool newbasis = false) noexcept;
  
    /// subtract
    status_t
    subtract(const std::string& ts, const std::string& tn,
             const std::string& ss, const std::string& sn) noexcept;
    
    
    
    ////////////////////////
    /// vector measurement
    ////////////////////////
    
    /// simlilarity (unit distance)
    boost::optional<double>
    similarity(const std::string&, const std::string&,
               const std::string&, const std::string&) noexcept;

    /// inner product (overlap)
    boost::optional<double>
    overlap(const std::string&, const std::string&,
            const std::string&, const std::string&) noexcept;

    
    ////////////////////////
    /// space operations ///
    ////////////////////////
    
    bool
    destroy_space(const std::string&) noexcept;

    std::vector<std::string>
    get_named_spaces() noexcept;
    
    boost::optional<std::size_t>
    get_space_cardinality(const std::string&) noexcept;

    ///////////////////
    /// heap metrics //
    ///////////////////
    
    inline std::size_t heap_size() noexcept { return heap.get_size(); }
    inline std::size_t free_heap() noexcept { return heap.get_free_memory(); }
    inline bool check_heap_sanity() noexcept { return heap.check_sanity(); }
    inline bool can_grow_heap() noexcept { return (heap.get_size() < maxheap); }

  protected:
    
    /// get named symbol
    boost::optional<const space::symbol&>
    get_symbol(const std::string& space_name,
               const std::string& symbol_name) noexcept;

    
  private:
    
    //////////////////////
    /// heap utilities ///
    //////////////////////
    
    bool grow_heap_by(const std::size_t&) noexcept;
    
    bool compactify_heap() noexcept;
    
    
      // get space
    space* get_space_by_name(const std::string&); 

    /// database memoizes pointers to named spaces to optimize symbol resolution 
    space* ensure_space_by_name(const std::string&); 
    
    // get randomizer
    inline random::index_randomizer& randomidx(void) { return irand; }

    
  private:    
   
    ////////////////////
    // lifetime state //
    ////////////////////

    // db parameters
    std::size_t inisize;
    std::size_t maxheap;
    segment_t heap;
    const std::string heapimage;
    const bool compclose;      // compact on close?
    
    // randomizer init at construction
    random::index_randomizer irand;

    // are we trying to grow?
    volatile bool isexpanding; 
    // read through space cache
    std::map<const std::string, space*> spaces; // used space cache
 
  };
}
