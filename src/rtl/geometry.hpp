#pragma once
#include "manifold.cpp"

namespace sdm {
}


    // STET for now as may reuse this code - but see lastest disaptch based MP //
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
  */




