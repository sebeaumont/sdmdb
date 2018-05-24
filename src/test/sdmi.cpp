/***************************************************************************
 * sdmq main routine
 *
 * Copyright (c) Simon Beaumont 2012-2014 - All Rights Reserved.
 * See: LICENSE for conditions under which this software is published.
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
//#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

// we are really developing this
#include "../rtl/manifold.hpp"


// a wall clock timer with microsecond resolution

class timer {
  
public:

  // set the start time at construction
  timer() : start(clock_t::local_time()) {}

  const std::size_t get_elapsed_micros() {
    
    time_t now(clock_t::local_time());
    elapsed_t d = now - start;
    return d.total_microseconds();
  }

  // ostream printer
  friend std::ostream& operator<<(std::ostream& os, timer& t) {
    os << "[" << t.get_elapsed_micros() << "] ";
    return os;
  }

private:

  typedef boost::posix_time::ptime time_t;
  typedef boost::date_time::microsec_clock<time_t> clock_t;
  typedef boost::posix_time::time_duration elapsed_t;
  
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


// hack tokenizer for symbols

bool parse_symbol(const std::string& s,
                  const std::string& prefix,
                  std::list<std::string>& parsed) {
  
  typedef boost::char_separator<char> sep_t;
  typedef boost::tokenizer<sep_t> toz_t; 

  sep_t sep(":");
  toz_t tok(s, sep);
  
  for(toz_t::iterator j = tok.begin(); j != tok.end(); ++j) {
    std::string t(*j);
    boost::trim(t);
    parsed.push_back(t);
  }
  if (parsed.size() == 1) parsed.push_front(prefix);
  if (parsed.size() == 2) return true;
  else {
    std::cout << "cannot parse symbol: " << s << std::endl;
    return false;
  }
}
  
  
// quck and dirty file existence check avoiding boost::system/filesystem libraries

inline bool file_exists(const char *path) {
  return std::ifstream(path).good();
}

inline bool file_exists(std::string& path) {
  return std::ifstream(path).good();
}


using namespace sdm;


////////////////////////////////
// entry point and command line

int main(int argc, const char** argv) {

  namespace po = boost::program_options;
  using namespace sdm;
  
  // command line options
    
  std::size_t initial_size;
  std::size_t maximum_size;
  
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("heapimage", -1);

  std::string default_space;
  
  desc.add_options()
    ("help", "SDM runtime test utility")
    ("heapsize", po::value<std::size_t>(&initial_size)->default_value(700),
     "initial size of heap in Mbytes")
    ("maxheap", po::value<std::size_t>(&maximum_size)->default_value(700),
     "maximum size of heap in Mbytes")
    ("heapimage", po::value<std::string>(),
     "heap image name (should be a valid path)")
    ("prefix", po::value<std::string>(&default_space)->default_value("words"),
     "default space name for unqalified symbols");
  
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

  
  // create space manifold - database+
 
  manifold rts(initial_size * 1024 * 1024, maximum_size * 1024 * 1024, heapfile); 

  // see if we can find space names
  std::vector<std::string> spaces = rts.get_named_spaces();
  for (auto sn: spaces) {
    std::cout << sn << " #" << rts.get_space_cardinality(sn) << std::endl;
  }
  
  // main command loop

  
  std::string prompt("-> ");
  std::string input;
  
  std::cout << prompt;  

  // simple command processor
  while (std::getline(std::cin, input)) {

    boost::trim(input);
    
    std::vector<std::string> cv;
    tokenize_command(input, cv);

    if (cv.size() > 1) {  
      // assume we have a command and at least one argument
      
      // dispatch command

      if (boost::iequals(cv[0], "=")) {

        std::list<std::string> sym;
        
        if (parse_symbol(cv[1], default_space, sym)) {
          timer t;
          status_t s = rts.namedvector(sym.front(), sym.back());
          std::cout << t << sym.front() << ":" << sym.back() << " (" << s << ")" << std::endl;
        }        


      } else if (boost::iequals(cv[0], "<")) {
        
        // load symbols from file
        std::ifstream ins(cv[1]);
        
        if (ins.good()) {
          std::string fline;
          int n = 0;
          timer mytimer;
          
          while(std::getline(ins, fline)) {
            boost::trim(fline);
            std::list<std::string> sym;
            if (parse_symbol(fline, default_space, sym)) {
              auto sts = rts.namedvector(default_space, fline);
              if (sdm_error(sts)) {
                std::cout << "stopped loading due to error: " << sts << std::endl;
                break;
              }
              n++;
            } else {

            }
          }
          
          std::cout << mytimer << " loaded: " << n << std::endl; 

        } else {
          std::cout << "can't open: " << cv[1] << std::endl;
        }

        
      } else if (boost::iequals(cv[0], ">")) {
        ; // TODO NEXT dump vectors for a space or symbol
      
        
      } else if (boost::iequals(cv[0], "-")) {

        std::list<std::string> sym1;
        std::list<std::string> sym2;

        if (parse_symbol(cv[1], default_space, sym1) &&
            parse_symbol(cv[2], default_space, sym2)) {
          timer t;
          rts.subtract(sym1.front(), sym1.back(), sym2.front(), sym2.back());
          std::cout << t << rts.density(sym1.front(), sym1.back()).second << std::endl;
        }
        
      } else if (boost::iequals(cv[0], "^")) {

        std::list<std::string> sym1;
        std::list<std::string> sym2;

        if (parse_symbol(cv[1], default_space, sym1) &&
            parse_symbol(cv[2], default_space, sym2)) {
        
          timer t;
          rts.superpose(sym1.front(), sym1.back(), sym2.front(), sym2.back());
          std::cout << t << rts.density(sym1.front(), sym1.back()).second << std::endl;
        }
        
      } else if (boost::iequals(cv[0], "+")) {

        std::list<std::string> sym1;
        std::list<std::string> sym2;

        if (parse_symbol(cv[1], default_space, sym1) &&
            parse_symbol(cv[2], default_space, sym2)) {
        
          timer t;
          rts.superpose(sym1.front(), sym1.back(), sym2.front(), sym2.back(), true);
          std::cout << t << rts.density(sym1.front(), sym1.back()).second << std::endl;
        }
        
      } else if (boost::iequals(cv[0], "?")) {

        std::list<std::string> sym1;
        std::list<std::string> sym2;
        
        if (parse_symbol(cv[1], default_space, sym1) &&
            parse_symbol(cv[2], default_space, sym2)) {

          timer t;
          std::cout << t << rts.similarity(sym1.front(), sym1.back(),
                                           sym2.front(), sym2.back()).second
                    << std::endl;
        }
        
      } else if (boost::iequals(cv[0], ".")) {
          
        std::list<std::string> sym1;
        std::list<std::string> sym2;
        
        if (parse_symbol(cv[1], default_space, sym1) &&
            parse_symbol(cv[2], default_space, sym2)) {

          timer t;
          double o = rts.overlap(sym1.front(), sym1.back(), sym2.front(), sym2.back()).second;
          std::cout << t << o << std::endl;
        }
        
      } else std::cout << "! syntax error:" << input << std::endl;

      
    } else if (cv.size() > 0) {
      // default to search if no args
      std::list<std::string> sym;

      if (parse_symbol(cv[0], default_space, sym)) {
          timer t;
          auto ip = rts.prefix_search(sym.front(), sym.back());
          std::cout << t << std::endl;
          if (ip) std::copy((*ip).first, (*ip).second,
                            std::ostream_iterator<database::space::symbol>(std::cout, "\n"));
          else std::cout << sym.front() << ":" << sym.back() << " not found" << std::endl;
        }
    }
    
    std::cout << prompt;  
  }
  
  // goodbye from me and goodbye from him...
  std::cout << std::endl
            << (rts.check_heap_sanity() ? ":-)" : ":-(")
            << " heap size: " << (float) rts.heap_size() / (1024*1024)
            << " free: " << (float) rts.free_heap() / (1024*1024)
            << std::endl << "...bye" << std::endl;
  
  return 0;
  
}

