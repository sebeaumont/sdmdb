#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <map>

//#include <Eigen/Dense>

#include "sdmconfig.h"
#include "sdmtypes.h"

#include "../mms/symbol_space.hpp"
#include "../mms/ephemeral_vector.hpp"


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


    
    /// constructor for mapped image
    
    explicit manifold(const std::string&, const std::size_t = 0);

    // no copy or move semantics;

    manifold(const manifold&) = delete;
    manifold(manifold&&) = delete;
    const manifold& operator=(const manifold&) = delete;
    const manifold& operator=(manifold&&) = delete;

      
    struct point {
    
      std::string name;
      double density;
    
      /// constructor copy of symbol space data
      explicit
      point(const std::string& v, const double d) : name(v), density(d) {}
     };
    
    
    struct neighbour : public point {
      
      double similarity;
      double overlap;
      
      explicit
      neighbour(const std::string& v,
                const double d,
                const double s,
                const double o) : point(v, d), similarity(s), overlap(o) {}
      
      
      /// comparison operators for sorting w.r.t metric
      // in order to compute topology of nearest neighbours
      // to a given vector
      
      bool operator< (const neighbour& s) const {
        return similarity > s.similarity;
      }
      
      bool operator==(const neighbour& s) const {
        return name == s.name && similarity == s.similarity;
      }
      
      bool operator!=(const neighbour& s) const {
        return name != s.name || similarity != s.similarity;
      }

      // get rid of this debug
      friend std::ostream& operator<<(std::ostream& os, neighbour& p) {
        os <<  p.name << "\t" << p.similarity << "\t" << p.overlap << "\t" << p.density;
        return os;
      }
      
    };
  
    typedef std::vector<point> geometry;
    typedef std::vector<neighbour> topology;
    
    // xx plannning to use this for all semantic vector data in space for search etc.
    // might need a few types -- some will need to be gpu compatible.
    //typedef Eigen::Matrix<SDM_VECTOR_ELEMENT_TYPE, SDM_VECTOR_ELEMS, Eigen::Dynamic> matrix_t;


    sdm_status_t
    load_vector(const std::string& space,
                const std::string& name,
                sdm_vector_t vector);

    sdm_status_t
    load_elemental(const std::string& space,
                   const std::string& name,
                   sdm_sparse_t bits);



    // XXX attempting c and c++ versions here
    // XXX could inline all the c++ implementations and only build a C library

    // apply a metric to get a subset of the space
    /*
    // ? move to sdmlib for c-api
    sdm_status_t
    get_topology(const std::string& targetspace,
                 const sdm_vector_t& vector,
                 const sdm_size_t cub,
                 //const sdm_metric_t metric,
                 const double dlb,
                 const double dub,
                 const double mlb,
                 const double mub,
                 sdm_topology_t& top);

    */
    sdm_status_t
    get_topology(const std::string& targetspace,
                 const std::string& sourcespace,
                 const std::string& vectorname,
                 topology& topo,
                 const double dub = 0.5,
                 const double mlb = 0.5,
                 const sdm_size_t cub = -1);

    
    // wrapper type for non-heap allocated vectors  
    typedef mms::ephemeral_vector<SDM_VECTOR_ELEMENT_TYPE,
                                  SDM_VECTOR_ELEMS,
                                  space::vector_t> svector;
    //
    sdm_status_t
    get_topology(const std::string& targetspace,
                 const sdm_vector_t& vector,
                 topology& top,
                 const double dub = 0.5,
                 const double mlb = 0.5,
                 const sdm_size_t cub = -1);

    /* move to sdmlib c api 
    sdm_status_t
    get_geometry(const std::string&,
                 const sdm_size_t,
                 sdm_geometry_t);
    */
    
    /// get all points in a space
    sdm_status_t
    get_geometry(const std::string&,
                 geometry&);

    /// search for symbols starting with prefix XXX move to database class
    
    typedef std::pair<manifold::space::symbol_iterator,
                      manifold::space::symbol_iterator> symbol_list;
    std::pair<sdm_status_t, symbol_list>
    prefix_search(const std::string& space_name,
                  const std::string& symbol_prefix) noexcept;
    
    
    /////////////////////////
    /// vector properties ///
    /////////////////////////
    
    /// get vector density
    std::pair<const sdm_status_t, const double>
    density(const std::string& space_name,
            const std::string& vector_name) noexcept;


    /////////////////////////
    /// vector measurement //
    /////////////////////////
    
    /// simlilarity (unit distance)

    const std::pair<const sdm_status_t, const double>
    similarity(const std::string&, const std::string&,
               const std::string&, const std::string&) noexcept;

    /// inner product (overlap)
    
    const std::pair<const sdm_status_t, const double>
    overlap(const std::string&, const std::string&,
            const std::string&, const std::string&) noexcept;

    /// get spaces in manifold

    std::vector<std::string>
    get_named_spaces() noexcept;

    /// get space cardinality
    
    std::pair<sdm_status_t, std::size_t>
    get_space_cardinality(const std::string&) noexcept;

  protected:

    inline space*
    get_space_by_name(const std::string& name) noexcept {
      auto it = spaces.find(name);
      return (it == spaces.end()) ? nullptr : it->second;
    }
   

    /// access cache of pointers to named spaces to optimize symbol lookup
    std::pair<sdm_status_t, space*> ensure_space_by_name(const std::string&); 
    

  protected:

    const std::string heapimage;
    const std::size_t inisize;
    segment_t  heap;

    // read through space cache
    std::map<const std::string, space*> spaces; // run time space index
    // todo read through toppology cache

  };

}



