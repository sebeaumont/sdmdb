#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/optional.hpp>
#include <map>

#include "../mms/feature_space.hpp"

using namespace sdm::mms;
typedef bip::managed_mapped_file segment_t;
  
// this is mms implementation (fully featured! :)
typedef feature_space<unsigned long, 256, 16, segment_t> space;
