// unit tests for runtime library
// copyright (c) 2015 Simon Beaumont. All Rights Reserved.

#include <cstdio>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE load_symbols
#include <boost/test/included/unit_test.hpp>

#include "rtl/database.hpp"

using namespace sdm;

// sizing
const std::size_t ini_size = 700 * 1024 * 1024;
const std::size_t max_size = 700 * 1024 * 1024;
const std::string image = "testheap.img";
const std::string test_lexicon = "/usr/share/dict/words";
const std::string test_space1 = "TESTSPACE";


// create and destroy rts
struct database_setup {
  database rts;
  database_setup () : rts(image, ini_size, max_size) {
    BOOST_TEST_MESSAGE("setup database");
  }
  ~database_setup () {
    // delete heapimage
    remove(image.c_str());
    BOOST_TEST_MESSAGE("cleanup database");
  }
};


BOOST_FIXTURE_TEST_SUITE(database_library, database_setup)

BOOST_AUTO_TEST_CASE(rts_init)
{
  BOOST_CHECK_EQUAL(rts.heap_size(), ini_size);
  BOOST_REQUIRE_LT(rts.free_heap(), rts.heap_size());
  BOOST_REQUIRE(rts.check_heap_sanity());
}

BOOST_AUTO_TEST_CASE(rts_load_vectors) {
  std::ifstream ins(test_lexicon);
  BOOST_REQUIRE(ins.good());
  
  std::string fline;
  int loaded = 0;
  int onlyload = 10000;
  
  while(std::getline(ins, fline) && loaded < onlyload) {
    boost::trim(fline);
    auto s = rts.namedvector(test_space1, fline);
    if (!sdm_error(s)) loaded++;
  }
  
  // get cardinality of space
  auto card = rts.get_space_cardinality(test_space1);
  
  BOOST_REQUIRE(!sdm_error(card.first));
  BOOST_CHECK_EQUAL(card.second, loaded);
  
  BOOST_TEST_MESSAGE("loaded: " << loaded << " will now prefix search (all)");
  
  // lookup all vectors
  auto sit = rts.prefix_search(test_space1, "");
  if (!sdm_error(sit.first)) for (auto it = sit.second.first; it != sit.second.second; ++it) {
    //std::cout << *it << std::endl;
    loaded--;
  }

  BOOST_CHECK_EQUAL(loaded, 0);
}


BOOST_AUTO_TEST_CASE(rts_search_empty_space) {

  auto card = rts.get_space_cardinality(test_space1);
  if (!sdm_error(card.first)) BOOST_TEST_MESSAGE(test_space1 << " #" << card.second);
  else BOOST_TEST_MESSAGE("cardinality: " << test_space1 << " no space found!");
  
  int found = 0;
  // lookup all vectors
  auto sit = rts.prefix_search(test_space1, "");
  if (!sdm_error(sit.first)) for (auto it = sit.second.first; it != sit.second.second; ++it) {
    //std::cout << *it << std::endl;
    found++;
  }

  BOOST_CHECK_EQUAL(found, 0);
    
}


BOOST_AUTO_TEST_SUITE_END()
