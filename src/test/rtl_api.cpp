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
  manifold db;
  manifold_setup () : db(ini_size, max_size, image) {
    BOOST_TEST_MESSAGE("setup manifold");
  }
  ~manifold_setup () {
    // delete heapimage
    remove(image.c_str());
    BOOST_TEST_MESSAGE("cleanup manifold");
  }
};


BOOST_FIXTURE_TEST_SUITE(manifold_api, manifold_setup)


BOOST_AUTO_TEST_CASE(rtl_api) {

  status_t s = db.superpose("names","Simon","names","Beaumont");
  BOOST_REQUIRE(!sdm_error(s));
  s = db.superpose("names","Beaumont","names","Simon");
  BOOST_REQUIRE(!sdm_error(s));
  s = db.superpose("names","Beaumont","names","Natasha");
  BOOST_REQUIRE(!sdm_error(s));
  s = db.superpose("names","Beaumont","names","Joshua");
  BOOST_REQUIRE(!sdm_error(s));
  s = db.superpose("names","Beaumont","names","Oliver");
  BOOST_REQUIRE(!sdm_error(s));
  s = db.superpose("names","Beaumont","names","Laura");
  BOOST_REQUIRE(!sdm_error(s));
  
  auto d = db.density("names", "Beaumont");
  BOOST_REQUIRE(!sdm_error(d.first));
  BOOST_REQUIRE(d.second > 0.004); 

  
  // get cardinality of space
  auto card = db.get_space_cardinality("names");
  
  BOOST_REQUIRE(!sdm_error(card.first));
  BOOST_CHECK_EQUAL(card.second, 6);

  manifold::topology_t t;
  status_t sts = db.get_topology("names", card.second, t);
  BOOST_REQUIRE(!sdm_error(sts));

  BOOST_REQUIRE(t.size() == 6);

  // ...
  
}


BOOST_AUTO_TEST_SUITE_END()
