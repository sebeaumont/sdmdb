// unit tests for runtime library
// copyright (c) 2015 Simon Beaumont. All Rights Reserved.

#include <cstdio>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE manifold_api
#include <boost/test/included/unit_test.hpp>

// we need database i/f to add data but we test manifold api!
#include "rtl/database.hpp"

using namespace sdm;

// sizing
const std::size_t ini_size = 700 * 1024 * 1024;
const std::size_t max_size = 700 * 1024 * 1024;
const std::string image = "testheap.img";
const std::string test_space1 = "TESTSPACE";
const std::string test_lexicon = "/usr/share/dict/words";


// create and destroy manifold 
struct manifold_setup {
  database rts;
  manifold_setup () : rts(image, ini_size, max_size) {
    BOOST_TEST_MESSAGE("setup manifold");
  }
  ~manifold_setup () {
    // delete heapimage
    remove(image.c_str());
    BOOST_TEST_MESSAGE("cleanup manifold");
  }
};


BOOST_FIXTURE_TEST_SUITE(manifold_api, manifold_setup)


BOOST_AUTO_TEST_CASE(rts_get_geometry) {
  std::ifstream ins(test_lexicon);
  BOOST_REQUIRE(ins.good());
  
  std::string fline;
  int loaded = 0;
  int onlyload = 10000;
  
  while(std::getline(ins, fline) && loaded < onlyload) {
    boost::trim(fline);
    auto s = rts.namedvector(test_space1, fline);
    if (!sdm_error(s)) {
      loaded++;
    } else {
      BOOST_TEST_MESSAGE("error in namedvector: " << s);
    }
  }
  
  // get cardinality of space
  auto card = rts.get_space_cardinality(test_space1);
  
  BOOST_REQUIRE(!sdm_error(card.first));
  BOOST_CHECK_EQUAL(card.second, loaded);

  database::geometry g;
  // some changes on allocation here we will expect caller to allocate
  // space but we will take a limit on vector caridnality for security.
  // also overload this method to a buffer (aliged pointer to an element_t not void*)
  // this will ease of cffi
  sdm_status_t sts = rts.get_geometry(test_space1, g);
  BOOST_CHECK_EQUAL(sts, AOK);

}


BOOST_AUTO_TEST_SUITE_END()
