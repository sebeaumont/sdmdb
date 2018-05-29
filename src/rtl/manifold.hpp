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
  
  class manifold {

    // memory manager for spaces within the memory mapped image
    typedef bip::managed_mapped_file segment_t;

    // shared file mapping logic
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
    
    // a reference to the underlying database to populate manifold spaces
    // we may want to unmap database spaces from our address space once we have populated cache

    
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

    /// get space
    inline space* get_space_by_name(const std::string& name) noexcept {
      auto it = spaces.find(name);
      return (it == spaces.end()) ? nullptr : it->second;
    }


  protected:
    /// database memoizes pointers to named spaces to optimize symbol lookup
    std::pair<status_t, space*> ensure_space_by_name(const std::string&); 
    

  protected:
    const std::string heapimage;
    const std::size_t inisize;
    segment_t  heap;

    // read through space cache
    std::map<const std::string, space*> spaces; // run time space index


  };



  /*
  struct point {
    
    std::string name;
    double metric;
    double density;
    
    point(const std::string& v,
          const double s,
          const double d) : name(v), metric(s), density(d) {}
    
    /// comparison operators for sorting
    bool operator< (const point& s) const {
      return metric > s.metric;
    }
    
    bool operator==(const point& s) const {
      return name == s.name && metric == s.metric;
    }
    
    bool operator!=(const point& s) const {
      return name != s.name || metric != s.metric;
    }
    
    friend std::ostream& operator<<(std::ostream& os, point& p) {
      os <<  p.name << "\t" << p.metric << "\t" << p.density;
      return os;
    }
      
  };
  */  

  /*

  //////////////////////////////////////
  /// computed neighbourhood of a vector
  
  
  ///////////////////////////////////////
  /// compute neighbourhood of a vector
  
  inline const topology neighbourhood(const vector_t& u,
                                      const double p,
                                      const double d,
                                      const std::size_t n) {
    
    const std::size_t m = vectors->size();
    // allocate working memory - TODO try this on stack
    auto work = new double[m*2];
    
    //// parallel block ////
#ifdef HAVE_DISPATCH
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
    dispatch_apply(m, queue, ^(std::size_t i) {
        work[i*2] = (*vectors)[i].density();
        work[i*2+1] = u.metric((*vectors)[i]);
      });
    // XXXX ARC forbids release XXXXX dispatch_release(queue);
    
#else
#pragma omp parallel for 
    for (std::size_t i=0; i < m; ++i) {
      work[i*2] = (*vectors)[i].density();
      work[i*2+1] = u.metric((*vectors)[i]);
    }
#endif
    //// end parallel block ////
    
    topology topo;
    topo.reserve(m); // ??? hmm is there a statistic here?
    
    // filter work array
    for (std::size_t i=0; i < m; ++i) {
      double rho = work[i*2];
      double sim = work[i*2+1];
      // apply p-d-filter
      if (rho <= d && sim >= p) {
        // XXX... avoid string copy here return symbol ref?
        topo.push_back(point((*this)[i].name(), sim, rho));
      }
    }
    
    delete[] work;
    // sort the scores in metric order 
    sort(topo.begin(), topo.end());
    const std::size_t ns = topo.size();
    
    // chop off uneeded tail
    topo.erase(topo.begin() + ((n < ns) ? n : ns), topo.end()); 
    return topo;
  }
  
  
  // overlap based vectors TODO use a c++ functor for metric 
  
  inline const topology neighbourhood2(const vector& u,
                                       const double p,
                                       const double d,
                                       const std::size_t n) {
    // TODO no copy for simple case?
    return neighbourhood2(ephemeral_vector_t(u), p, d, n);
  }
  
  //////////////////////////////////////////////////////////////////
  /// compute neighbourhood of a vector based on overlap rather than 
  /// hamming distance.
  /// TODO might combine metrics into points or better pass in the metric function
  
  
  inline const topology neighbourhood2(const ephemeral_vector_t& u,
                                       const double p,
                                       const double d,
                                       const std::size_t n) {

    // XXX need to iterate over a named space now    
    const std::size_t m = vectors->size();
    
    // allocate working memory - TODO try this on stack
    auto work = new double[m*2];
    
    //// parallel block ////
#ifdef HAVE_DISPATCH
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
    dispatch_apply(m, queue, ^(std::size_t i) {
        work[i*2] = (*vectors)[i].density();
        work[i*2+1] = u.overlap((*vectors)[i]);
      });
    // XXXX ARC forbids release XXXXX dispatch_release(queue);
    
#else
#pragma omp parallel for 
    for (std::size_t i=0; i < m; ++i) {
      work[i*2] = (*vectors)[i].density();
      work[i*2+1] = u.overlap((*vectors)[i]);
    }
#endif
    //// end parallel block ////
    
    topology topo;
    topo.reserve(m); // ??? hmm is there a statistic here?
    
    // filter work array
    for (std::size_t i=0; i < m; ++i) {
      double rho = work[i*2];
      double sim = work[i*2+1];
      // apply p-d-filter
      if (rho <= d && sim >= p) {
        // XXX... avoid string copy here return symbol ref?
        topo.push_back(point((*this)[i].name(), sim, rho));
      }
    }
    
    delete[] work;
    // sort the scores in metric order 
    sort(topo.begin(), topo.end());
    const std::size_t ns = topo.size();
    
    // chop off uneeded tail
    topo.erase(topo.begin() + ((n < ns) ? n : ns), topo.end()); 
    return topo;
  }
  */
    

}



