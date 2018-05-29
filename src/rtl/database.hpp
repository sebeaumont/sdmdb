// Copyright (c) 2015 Simon Beaumont - All Rights Reserved
// runtime.hpp - runtime api interface

#pragma once
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/optional.hpp>
#include <map>

#include "manifold.hpp"
#include "../mms/symbol_space.hpp"
#include "../util/fast_random.hpp"


#include "sdmconfig.h"
#include "sdmstatus.h"

#include <iostream>

namespace sdm {

  namespace bip = boost::interprocess;

  /***********************************************************************
   ** Database type provides the API for the SDM implementation
   ***********************************************************************/
  
  class database : public manifold {

    /// memory manager for spaces within the database
    typedef bip::managed_mapped_file segment_t;
    
  public:

    /// constructor to open or create file mapped heap r/w
    
    explicit database(const std::string& filepath,
                      const std::size_t initial_size,
                      const std::size_t max_size,
                      const bool compact=false);

    
    /// no copy or move semantics

    database() = delete;
    database(const database&) = delete;
    database(database&&) = delete;
    const database& operator=(const database&) = delete;
    const database& operator=(database&&) = delete;

    /// destructor will cautiously ensure all pages are flushed
    
    ~database();
    

    /////////////////////////////////////////////////////
    /// effectful learning operations on target vectors
    /////////////////////////////////////////////////////

    
    /// assert a named vector
        
    const status_t
    namedvector(const std::string& space_name,
                const std::string& symbol_name,
                // XX to do change type to a double p value default to 1
                const space::symbol::type type = space::symbol::type::normal) noexcept;

    
    /// add or superpose
    
    const status_t
    superpose(const std::string& ts, const std::string& tn,
              const std::string& ss, const std::string& sn,
              const int shift = 0) noexcept;

    /// batch superpose several symbols from source space

    const status_t
    superpose(const std::string& ts, const std::string& tn,
              const std::string& ss, const std::vector<const std::string>& sns,
              const std::vector<const int> shifts) noexcept;
    
    /// subtract

    const status_t
    subtract(const std::string& ts, const std::string& tn,
             const std::string& ss, const std::string& sn) noexcept;
    
    
    
    ////////////////////////
    /// space operations ///
    ////////////////////////
    
    bool
    destroy_space(const std::string&) noexcept;

    ///////////////////
    /// heap metrics //
    ///////////////////
    
    inline std::size_t heap_size() noexcept { return heap.get_size(); }
    inline std::size_t free_heap() noexcept { return heap.get_free_memory(); }
    inline bool check_heap_sanity() noexcept { return heap.check_sanity(); }
    inline bool can_grow_heap() noexcept { return (heap.get_size() < maxheap); }


    ////////////////////////////////////////////
    /// internal functions typically inlined ///
    ////////////////////////////////////////////
    
  private:

    inline std::pair<status_t, space::symbol&>
    ensure_mutable_symbol(const std::string&,
                          const std::string&,
                          const space::symbol::type);

    inline std::pair<status_t, const space::symbol*>
    ensure_symbol(const std::string&,
                  const std::string&,
                  const space::symbol::type);

  private:
    
    //////////////////////
    /// heap utilities ///
    //////////////////////
    
    bool grow_heap_by(const std::size_t&) noexcept;
    
    bool compactify_heap() noexcept;
    
    /// get randomizer
    inline random::index_randomizer& randomidx(void) { return irand; }

    
  private:    
   
    ////////////////////
    // lifetime state //
    ////////////////////

    // db parameters
    std::size_t maxheap;
    const bool compclose;
    
    // randomizer init at construction
    random::index_randomizer irand;

    // are we trying to grow?
    volatile bool isexpanding;
 
  };
}
