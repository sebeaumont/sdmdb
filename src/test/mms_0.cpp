/***************************************************************************
 * basic paranoia test case of mms
 *
 * Copyright (c) Simon Beaumont 2012-2014 - All Rights Reserved.
 * See: LICENSE for conditions under which this software is published.
 ***************************************************************************/
#include <cstdio>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>


#define BOOST_TEST_MODULE mms-0
#include <boost/test/included/unit_test.hpp>
#include "mms/symbol_space.hpp"

namespace bip = boost::interprocess;
    
const std::size_t requested_size = 2 * 1024 * 1024; // bytes
const std::string tablename = "woobongaruru";
const std::string heapfile = "vpsace-0.img";
const std::string v0 = "vector-0";

typedef bip::managed_mapped_file segment_t;
typedef molemind::sdm::mms::symbol_space<unsigned long, 256, 16, segment_t> space_t;

//int main(int argc, char** argv) {


// test context
struct test_setup {
  segment_t segment;
  space_t mms;
  
  test_setup() : segment(bip::open_or_create, heapfile.c_str(), requested_size),
                 mms(tablename, segment) {}
  
  ~test_setup() { remove(heapfile.c_str()); }
};

/*
  test_setup ctx;
  std::cout << ctx.mms << std::endl; 
  ctx.mms.insert(v0);
  std::cout << ctx.mms << std::endl;

  return 0;
}
*/

BOOST_FIXTURE_TEST_SUITE(mms_0, test_setup)



BOOST_AUTO_TEST_CASE(insert_vector) {
  std::vector<std::size_t> basis; // XXX test this too! now
  basis.push_back(12);
  basis.push_back(43);
  basis.push_back(99);
  
  std::cout << mms << std::endl;
  //BOOST_TEST_MESSAGE(msg);
  mms.insert(v0, basis);
  std::cout << mms << std::endl;
  //BOOST_TEST_MESSAGE(msg);
  // 5. retrieve vector
  BOOST_REQUIRE(mms.get_symbol_by_name(v0));
}

BOOST_AUTO_TEST_SUITE_END()

  
