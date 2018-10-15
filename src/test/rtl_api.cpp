// unit tests for runtime library
// copyright (c) 2015 Simon Beaumont. All Rights Reserved.

#include <cstdio>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE manifold_api
#include <boost/test/included/unit_test.hpp>

// need database api to add data
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
  database db;
  manifold_setup () : db(image, ini_size, max_size) {
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

  sdm_status_t s = db.superpose("names","Simon","names","Beaumont");
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

  // 
  database::geometry g;
  sdm_status_t sts = db.get_geometry("names", g);
  BOOST_REQUIRE(!sdm_error(sts));


  // vector load ...
  sdm_vector_t v;
  sts = db.load_vector("names", "Beaumont", v);
  BOOST_REQUIRE(!sdm_error(sts));

  // get a 
  // topo...
    
  database::topology t;
  t.reserve(20);
  sts = db.get_topology("names", v, 10, 0.5, 0.5, t);
  BOOST_REQUIRE(!sdm_error(sts));
  BOOST_REQUIRE(t.size() > 2);

  // load elemental...
  
  sdm_sparse_t e;
  sts = db.load_elemental("names", "Beaumont", e);
  BOOST_REQUIRE(!sdm_error(sts));
  
  // vector store...
}


BOOST_AUTO_TEST_SUITE_END()
