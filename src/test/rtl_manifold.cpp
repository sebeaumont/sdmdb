// unit tests for runtime library
// copyright (c) 2015 Simon Beaumont. All Rights Reserved.

#include <cstdio>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE manifold_api
#include <boost/test/included/unit_test.hpp>

#include "rtl/manifold.hpp"

using namespace sdm;

// sizing
const std::size_t ini_size = 700 * 1024 * 1024;
const std::size_t max_size = 700 * 1024 * 1024;
const std::string image = "testheap.img";
const std::string test_space1 = "TESTSPACE";
const std::string test_lexicon = "/usr/share/dict/words";


// create and destroy manifold 
struct manifold_setup {
  manifold rts;
  manifold_setup () : rts(ini_size, max_size, image) {
    BOOST_TEST_MESSAGE("setup manifold");
  }
  ~manifold_setup () {
    // delete heapimage
    remove(image.c_str());
    BOOST_TEST_MESSAGE("cleanup manifold");
  }
};


BOOST_FIXTURE_TEST_SUITE(manifold_api, manifold_setup)


BOOST_AUTO_TEST_CASE(rts_get_topology) {
  std::ifstream ins(test_lexicon);
  BOOST_REQUIRE(ins.good());
  
  std::string fline;
  int loaded = 0;
  int onlyload = 1000;
  
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
  
  BOOST_REQUIRE(card);
  BOOST_CHECK_EQUAL(*card, loaded);

  manifold::topology_t t;
  status_t sts = rts.get_topology(test_space1, t);
  BOOST_CHECK_EQUAL(sts, AOK);

}


BOOST_AUTO_TEST_SUITE_END()
