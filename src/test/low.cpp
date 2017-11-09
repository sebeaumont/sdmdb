/***************************************************************************
 * main.cpp symbol table test utility - part of the Sdm qdsm environment.
 *
 * Copyright (c) Simon Beaumont 2012-2014 - All Rights Reserved.
 * See: LICENSE for conditions under which this software is published.
 ***************************************************************************/

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/optional.hpp>

#include "symbolic_space.hpp"
#include "elemental_space.hpp"
#include "semantic_space.hpp"
#include "feature_space.hpp"

// wall clock timer

class timer {
  
public:

  timer(const std::string& name) : name(name), start(clock_t::local_time()) {}

  const std::size_t get_elapsed_micros() {
    
    time_t now(clock_t::local_time());
    elapsed_t d = now - start;
    return d.total_microseconds();
  }

  friend std::ostream& operator<<(std::ostream& os, timer& t) {
    os << "[" << t.get_elapsed_micros() << "] ";
    return os;
  }

private:

  typedef boost::posix_time::ptime time_t;
  typedef boost::date_time::microsec_clock<time_t> clock_t;
  typedef boost::posix_time::time_duration elapsed_t;
  
  std::string name;
  time_t start;
};


// tokenzier for input line

void tokenize_command(const std::string& s, std::vector<std::string>& o) {
  typedef boost::escaped_list_separator<char> ls_t;
  typedef boost::tokenizer<ls_t> toz_t;
  
  ls_t els("\\"," \t","\"\'");
  toz_t tok(s, els);
  
  for(toz_t::iterator j = tok.begin(); j != tok.end(); ++j) {
    std::string t(*j);
    boost::trim(t);
    o.push_back(t);
  }
}

// quck and dirty file existence check avoiding boost::system/filesystem libraries

inline bool file_exists(const char *path) {
  return std::ifstream(path).good();
}

inline bool file_exists(std::string& path) {
  return std::ifstream(path).good();
}


////////////////////////////////
// entry point and command line

int main(int argc, const char** argv) {

  namespace po = boost::program_options;
  namespace gs = sdm::mms;
  namespace bip = boost::interprocess;
    
  std::size_t requested_size;
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("name", -1);

  desc.add_options()
    ("help", "Sdm qdsm symbol table test utility")
    ("size", po::value<std::size_t>(&requested_size)->default_value(0),
     "requested size for table in Mbytes")
    ("name", po::value<std::string>(),
     "name (should be a valid path)");

  po::variables_map opts;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), opts);
  po::notify(opts);
  
  if (opts.count("help")) {
    std::cout << desc <<  std::endl;
    return 1;
  } else if (!opts.count("name")) {
    std::cout << "table name is required!" << std::endl;
    return 3;
  }

  std::string tablename(opts["name"].as<std::string>());

  if (!file_exists(tablename) && requested_size == 0) {
    std::cout << "cannot create new table: " << tablename
              << " with: " << requested_size << " bytes" << std::endl;
    return 5;
  }

  ///// this is where we develop test runtime operations

  // convert to bytes
  requested_size = requested_size * (1024 * 1024); 
  
  typedef bip::managed_mapped_file segment_t;

  // xxx todo sizing and stuff...
  segment_t segment(bip::open_or_create, "sdm.dat", requested_size);

  typedef gs::feature_space<unsigned long, 256, 16, segment_t> space_t;
  space_t mytable(tablename, segment);

  //gs::elemental_space<unsigned long, 32, segment_t> table2("foobar", segment);
  
  // create a space table
  //space_t mytable(tablename, segment);
  
  std::cout << mytable << std::endl;
  
  //int lastindex = mytable.entries();
  
  std::string prompt("Î¨> ");
  std::string input;
  
  std::cout << prompt;  
  // simple command processor
  while (std::getline(std::cin, input)) {

    boost::trim(input);
    
    std::vector<std::string> cv;
    tokenize_command(input, cv);

    if (cv.size() > 1) {  
      // assume we have a command
      
      // dispatch command
      if (boost::iequals(cv[0], "=")) {

        // N.B. AFAIK insertion suceeds even if the symbol already exists in the table
        mytable.insert(cv[1]);

        // lookup the inserted symbol
        if (auto sym = mytable.get(cv[1]))
          std::cout << *sym << std::endl;
        else
          std::cout << cv[1] << ": not found after insert (bug?)" << std::endl;
        
      } else if (boost::iequals(cv[0], "<")) {
        
        // load symbols from file
        std::ifstream ins(cv[1]);
        
        if (ins.good()) {
          std::string fline;
          int n = 0;
          timer mytimer("load timer");
          
          while(std::getline(ins, fline)) {
            boost::trim(fline);
            mytable.insert(fline);
            n++;
          }
          std::cout << mytimer << " loaded: " << n << std::endl; 
        } else {
          std::cout << "can't open: " << cv[1] << std::endl;
        }
        
      } else if (boost::iequals(cv[0], ">")) {
        ; // export file

      } else if (boost::iequals(cv[0], ".")) {
        // array access to space
        for (size_t i=0; i < mytable.entries();  ++i) {
          std::cout << mytable[i] << std::endl;
        }

      } else
        std::cout << "syntax error:" << input << std::endl;

      
    } else if (cv.size() > 0) {
      // default to search if no args
      auto ip = mytable.search(cv[0]);
      std::copy(ip.first, ip.second, std::ostream_iterator<space_t::vector>(std::cout, "\n"));
    }
    
    std::cout << prompt;  
  }
  
  // goodbye from me and goodbye from him...
  std::cout << std::endl << mytable << std::endl << "goodbye" << std::endl;
  
  return 0;
  
}
