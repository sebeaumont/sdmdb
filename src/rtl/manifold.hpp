#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <map>

#include "sdmconfig.h"
#include "sdmstatus.h"

#include "../mms/symbol_space.hpp"
#include "../mms/bitvector.hpp"


namespace sdm {
  
  namespace bip = boost::interprocess;    


  /// a manfiold in two senses: a multiplicity of locally "euclidean" vector spaces ;-)
  /// in some memory somewhere: could be main, gpu, fpga co-processor.
  /// the underlying spaces are taken from a read only mapped image data (mmf)
  
  class manifold {

    // N.B. need this in remap when growing database.
    
  protected:
    
    /// memory manager for spaces within the memory mapped image
    typedef bip::managed_mapped_file segment_t;

  private:
    
    /// file mapping logic
    inline segment_t mapfile(const std::string& mmf, const size_t size) {
      // create heap
      return  (size > 0)
        ? segment_t(bip::open_or_create, mmf.c_str(), size)
        : segment_t(bip::open_read_only, mmf.c_str());
    }

    
  public:

    // fundemental implementation types
    
    typedef mms::bitvector<SDM_VECTOR_ELEMENT_TYPE, SDM_VECTOR_ELEMS> vector_t;
    typedef std::vector<vector_t> topology_t;    
    
    /// type of space implementation determines the type and number of elements and sparsity
    
    typedef mms::symbol_space<SDM_VECTOR_ELEMENT_TYPE,
                              SDM_VECTOR_ELEMS,
                              SDM_VECTOR_BASIS_SIZE,
                              segment_t> space;

    
    /// constructor for mapped image
    
    explicit manifold(const std::string&, const std::size_t = 0);

    // no copy or move semantics;

    manifold(const manifold&) = delete;
    manifold(manifold&&) = delete;
    const manifold& operator=(const manifold&) = delete;
    const manifold& operator=(manifold&&) = delete;

  
    /// get vectors for a space
    status_t get_topology(const std::string&, std::size_t, topology_t&); 
    

    /// search for symbols starting with prefix
    
    typedef std::pair<manifold::space::symbol_iterator,
                      manifold::space::symbol_iterator> symbol_list;
    
    std::pair<status_t, symbol_list>
    prefix_search(const std::string& space_name,
                  const std::string& symbol_prefix) noexcept;
    
    
    /////////////////////////
    /// vector properties ///
    /////////////////////////
    
    /// get vector density
    std::pair<const status_t, const double>
    density(const std::string& space_name,
            const std::string& vector_name) noexcept;



    /////////////////////////
    /// vector measurement //
    /////////////////////////
    
    /// simlilarity (unit distance)

    const std::pair<const status_t, const double>
    similarity(const std::string&, const std::string&,
               const std::string&, const std::string&) noexcept;

    /// inner product (overlap)
    
    const std::pair<const status_t, const double>
    overlap(const std::string&, const std::string&,
            const std::string&, const std::string&) noexcept;


    /// get spaces in manifold

    std::vector<std::string>
    get_named_spaces() noexcept;

    /// get space cardinality
    
    std::pair<status_t, std::size_t>
    get_space_cardinality(const std::string&) noexcept;


  protected:

    /// get space pointer
    inline space* get_space_by_name(const std::string& name) noexcept {
      auto it = spaces.find(name);
      return (it == spaces.end()) ? nullptr : it->second;
    }

    /// database memoizes pointers to named spaces to optimize symbol lookup
    std::pair<status_t, space*> ensure_space_by_name(const std::string&); 
    

  protected:

    const std::string heapimage;
    const std::size_t inisize;
    segment_t  heap;

    // read through space cache
    std::map<const std::string, space*> spaces; // run time space index


  };

}



