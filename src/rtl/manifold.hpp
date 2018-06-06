#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <map>

#include <Eigen/Dense>

#include "config.h"

#include "sdmconfig.h"
#include "sdmstatus.h"

#include "../mms/symbol_space.hpp"
#include "../mms/bitvector.hpp"


namespace sdm {
  
  namespace bip = boost::interprocess;    
  
  // XXX under very active experimental development

  /// a manfiold in two senses: a multiplicity of locally "euclidean" vector spaces ;-)
  /// in some memory somewhere: could be main, gpu, fpga co-processor.
  /// the underlying spaces are taken from a read only mapped image data (mmf)
  
  class manifold {

    
  protected:
    
    /// memory manager for spaces within the memory mapped image
    typedef bip::managed_mapped_file segment_t;

  private:
    
    /// file mapping logic
    inline segment_t mapfile(const std::string& mmf, const size_t size=0) {
      // create heap
      return  (size > 0)
        ? segment_t(bip::open_or_create, mmf.c_str(), size)
        : segment_t(bip::open_read_only, mmf.c_str());
    }

    
  public:

    /// Fundemental implementation types
    
    /// space implementation determines the type and number of elements and sparsity
    /// of vectors
    
    typedef mms::symbol_space<SDM_VECTOR_ELEMENT_TYPE,
                              SDM_VECTOR_ELEMS,
                              SDM_VECTOR_BASIS_SIZE,
                              segment_t> space;

    /// API types under construction TODO make these C friendly

    // use space dependent types to define these?
    // typedef mms::bitvector<SDM_VECTOR_ELEMENT_TYPE, SDM_VECTOR_ELEMS> vector_t;
    typedef SDM_VECTOR_ELEMENT_TYPE vector_t[SDM_VECTOR_ELEMS];

    typedef vector_t geometry_t [];    

    typedef Eigen::Matrix<SDM_VECTOR_ELEMENT_TYPE, SDM_VECTOR_ELEMS, Eigen::Dynamic> matrix_t;

    typedef std::vector<std::pair<std::size_t, double>>  topology_t;

    typedef enum {Similarity, Overlap} metric_t;

    typedef std::vector<std::size_t> sparse_t;

    
    /// constructor for mapped image
    
    explicit manifold(const std::string&, const std::size_t = 0);

    // no copy or move semantics;

    manifold(const manifold&) = delete;
    manifold(manifold&&) = delete;
    const manifold& operator=(const manifold&) = delete;
    const manifold& operator=(manifold&&) = delete;

    /// TODO return a symbol wholesale (serialized)
    /*
    status_t
    get_symbol(const std::string& space,
               const std::string& name,
               std::string& json);
    */

    status_t
    load_vector(const std::string& space,
                const std::string& name,
                vector_t vector);

    status_t
    load_element(const std::string& space,
                 const std::string& name,
                 sparse_t fp);
    
    /// measure a vector
    
    status_t
    get_topology(const std::string& targetspace,
                 const vector_t& vector,
                 const std::size_t cub,
                 const metric_t metric,
                 const double dlb,
                 const double dub,
                 const double mlb,
                 const double mub,
                 topology_t& top);
      
    /// get vectors for a space

    status_t
    get_geometry(const std::string&, std::size_t, geometry_t); 
    

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

    /// access cache of pointers to named spaces to optimize symbol lookup
    std::pair<status_t, space*> ensure_space_by_name(const std::string&); 
    

  protected:

    const std::string heapimage;
    const std::size_t inisize;
    segment_t  heap;

    // read through space cache
    std::map<const std::string, space*> spaces; // run time space index
    // todo read through toppology cache

  };

}



