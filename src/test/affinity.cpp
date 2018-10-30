/***************************************************************************
 * produce affinity matrix for a space
 *
 * See: LICENSE for conditions under which this software is published.
 ***************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include <boost/program_options.hpp>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

// local includes 
#include "../rtl/database.hpp"
#include "index_bimap.hpp"
#include "sparse_matrix.hpp"
#include "sparse_matrix_io.hpp"


#define B2MB(b_) ((double)(b_)/(1024*1024))

    
// quick and dirty file existence check

inline bool file_exists(const char *path) {
  return std::ifstream(path).good();
}

inline bool file_exists(const std::string& path) {
  return std::ifstream(path).good();
}


using namespace sdm;


////////////////////////////////
// entry point and command line

int main(const int argc, const char** argv) {

  namespace po = boost::program_options;
  using namespace sdm;
  
  // command line options
    
  std::size_t initial_size;
  std::size_t maximum_size;
  double metric_lb;
  double density_ub;
  
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("heapimage", -1);

  std::string space_name;
  
  desc.add_options()
    ("help", "SDM runtime test utility")
    ("heapsize", po::value<std::size_t>(&initial_size)->default_value(700),
     "initial size of heap in Mbytes")
    ("maxheap", po::value<std::size_t>(&maximum_size)->default_value(700),
     "maximum size of heap in Mbytes")
    ("metric_min", po::value<double>(&metric_lb)->default_value(0.5),
     "minimum value of metric")
    ("density_max", po::value<double>(&density_ub)->default_value(1.0),
     "maxiumum value of density")
    ("heapimage", po::value<std::string>(),
     "heap image name (should be a valid path)")
    ("space", po::value<std::string>(&space_name)->default_value("words"),
     "name of space to extract topology");
  
  po::variables_map opts;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), opts);
  po::notify(opts);
  
  if (opts.count("help")) {
    std::cout << desc <<  std::endl;
    return 1;
  } else if (!opts.count("heapimage")) {
    std::cout << "heap image file is required!" << std::endl;
    return 3;
  }

  std::string heapfile(opts["heapimage"].as<std::string>());

  
  // open database image mmf
 
  database rts(heapfile, initial_size * 1024 * 1024, maximum_size * 1024 * 1024); 

  /*
  // see if we can find named spaces in image
  std::vector<std::string> spaces = rts.get_named_spaces();
  for (auto sn: spaces) {
    auto card = rts.get_space_cardinality(sn);
    std::cout << sn << " #" << card.second << std::endl;
  }
  */
  
  //  get space bail if error
  auto r = rts.get_space_cardinality(space_name);
        
  if (!sdm_error(r.first)) {
    
    std::cout << space_name << " cardinality: " << r.second << std::endl;

    // allocate geometry
    database::geometry g;
    g.reserve(r.second);
    
    // get all vectors in the space
    sdm_status_t sts = rts.get_geometry(space_name, g);

    if (!sdm_error(sts)) {
      
      // produce bimap from vectors/point in geometry and serialise to file stream 
        
      // allocate data structures
      index_bimap<std::string> feature_map;
      triplet_vec triplets; // i,j,v triplets for sparse matrix

      // for all points in space
      for (database::point p : g) {
        
        std::cout << "." << std::flush;
        // get index for name
        std::size_t i = feature_map.ensure(p.name);
        // get the topology/level set for this point/vector
        manifold::topology level_set;
        level_set.reserve(r.second);
        // carefully provide all parameters
        sts = rts.get_topology(space_name, space_name, p.name, level_set,
                               density_ub, 0., metric_lb, r.second);
        if (!sdm_error(sts)) {
          for (database::neighbour n : level_set) {
            std::size_t j = feature_map.ensure(n.name);
            triplets.push_back(triplet(i, j, n.similarity)); // XXX could try overlap metric
          }
        } else {
          std::cerr << "failed to get topology: " << sts << " for point: " << p.name << std::endl;
          return 9;
        }
      }

      // create sparse matrix from triplets
      sparse_matrix A(feature_map.size(), feature_map.size());
      A.setFromTriplets(triplets.begin(), triplets.end());
      
      // serialize index map to file
      {
        std::ofstream idxf(space_name + ".idx");        
        if (idxf.good() && feature_map.serialize(idxf))
          idxf.close();
      }

      // serialize matrix to file
      {
        std::ofstream matf(space_name + ".mat");
        // N.B. Affinity is symmetric!
        if (matf.good() && serialize_matrix(A, matf))
          matf.close();
      }

    } else {
      std::cerr << "failed to get geometry: " << sts << " for space: " << space_name << std::endl;
      return 7;
    }
    
  } else {
    std::cerr << "failed to find space: " << r.first << " for space: " << space_name << std::endl;
    return 5;
  }

  // done!
  std::cout << heapfile << ": " << (rts.check_heap_sanity() ? "✔" : "✘")
            << " heap size: "   << B2MB(rts.heap_size())
            << " free: "        << B2MB(rts.free_heap()) << std::endl;

  return 0;
  
}


