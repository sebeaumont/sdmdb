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
#include "../rtl/database.hpp"
// and this temporary vectors
#include "../mms/ephemeral_vector.hpp"


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


using namespace molemind::sdm;

// how to subvert our encapsulation with leaky abstractions... ;-)
// and test the evolving API...

static inline boost::optional<database::space::vector&> get_vector(database& db,
                                                                   const std::string& space_name,
                                                                   const std::string& name) {
  auto space = db.get_space_by_name(space_name);
  if (space)
    return space->get_vector_by_name(name);
  else
    return boost::none;
}

static inline boost::optional<const database::space::symbol&> get_symbol(database& db,
                                                                         const std::string& space_name,
                                                                         const std::string& name) {
  auto space = db.get_space_by_name(space_name);
  if (space)
    return space->get_symbol_by_name(name);
  else
    return boost::none;
}



////////////////////////////////
// entry point and command line

int main(int argc, const char** argv) {

  namespace po = boost::program_options;
  using namespace molemind::sdm;
  
  // command line options
    
  std::size_t initial_size;
  std::size_t maximum_size;
  
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("heap", -1);

  desc.add_options()
    ("help", "SDM runtime test utility")
    ("heapsize", po::value<std::size_t>(&initial_size)->default_value(700),
     "initial size of heap in Mbytes")
    ("maxheap", po::value<std::size_t>(&maximum_size)->default_value(700),
     "maximum size of heap in Mbytes")
    ("heap", po::value<std::string>(),
     "heap image name (should be a valid path)");
  
  po::variables_map opts;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), opts);
  po::notify(opts);
  
  if (opts.count("help")) {
    std::cout << desc <<  std::endl;
    return 1;
  } else if (!opts.count("heap")) {
    std::cout << "heap image is required!" << std::endl;
    return 3;
  }

  std::string heapfile(opts["heap"].as<std::string>());

  
  // create database with requirement
  database rts(initial_size * 1024 * 1024, maximum_size * 1024 * 1024, heapfile); 

  // XXX pro tem default space XXX FIX ME!!!
  const std::string default_spacename("words");
  
  // see if we can find space names
  std::vector<std::string> spaces = rts.get_named_spaces();
  for (unsigned i = 0; i < spaces.size(); ++i) {
    std::cout << i+1 << ":" << spaces[i];
    // now actually get the pointers
    auto spp = rts.get_space_by_name(spaces[i]);
    std::cout << "@" << spp << "#" << spp->entries() << std::endl;
  }
  
  // main command loop

  // what should this interface be called really?
  database::space::ephemeral_vector_t local_vector;
  
  std::string prompt("Ψ> ");
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

        // 
        status_t s = rts.ensure_symbol(default_spacename, cv[1]);

        // now lookup the inserted symbol
        if (auto sym = get_symbol(rts, default_spacename, cv[1]))
          std::cout << "[" << s << "] " << sym->name() << std::endl;
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
            //std::cout << "addv(" << default_spacename << ":" << fline << ")" << std::endl;
            rts.ensure_symbol(default_spacename, fline);
            n++;
          }
          std::cout << mytimer << " loaded: " << n << std::endl; 
        } else {
          std::cout << "can't open: " << cv[1] << std::endl;
        }
        
      } else if (boost::iequals(cv[0], ">")) {
        ; // export file
      
      /* TODO convert these to use abstract api */
        
      } else if (boost::iequals(cv[0], "/")) {
        // array access to space
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        // xxx
        if (v) {
          database::space::ephemeral_vector_t myvector(*v);
          
          std::cout << v->count() << " " << myvector.distance(*v) << std::endl;
        }
      } else if (boost::iequals(cv[0], "+")) {
        rts.superpose(default_spacename, cv[1], default_spacename, cv[2]);
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        //boost::optional<database::space::vector&> u = get_vector(rts, default_spacename, cv[2]);
        // optional guards? tee hee
        //v->superpose(*u);
        std::cout << v->count() << std::endl;
        
      } else if (boost::iequals(cv[0], "?")) {
        //rts.superpose(default_spacename, cv[1], default_spacename, cv[2]);
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        boost::optional<database::space::vector&> u = get_vector(rts, default_spacename, cv[2]);
        // optional guards? tee hee
        std::cout << v->similarity(*u) << std::endl;
        
      } else if (boost::iequals(cv[0], ".")) {
        //rts.superpose(default_spacename, cv[1], default_spacename, cv[2]);
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        boost::optional<database::space::vector&> u = get_vector(rts, default_spacename, cv[2]);
        // optional guards? tee hee
        std::cout << v->inner(*u) << std::endl;
        
      } else if (boost::iequals(cv[0], "!")) {
        // randomize the vector...
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        rts.randomize_vector(v, 0.001);
        std::cout << v->count() << std::endl;
      
      } else if (boost::iequals(cv[0], "%")) {
        // topo
        //boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        boost::optional<sdm::topology> t = rts.neighbourhood(default_spacename, default_spacename, cv[1], 0, 1, 20);
        if (t) for (auto p : *t)
          std::cout << p << std::endl;

       
      } else if (boost::iequals(cv[0], "1")) {
        // randomize the vector...
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        v->ones();
        std::cout << v->count() << std::endl;

      } else if (boost::iequals(cv[0], "0")) {
        // randomize the vector...
        boost::optional<database::space::vector&> v = get_vector(rts, default_spacename, cv[1]);
        v->zeros();
        std::cout << v->count() << std::endl;
    

      } else
        std::cout << "syntax error:" << input << std::endl;

      
    } else if (cv.size() > 0) {
      // default to search if no args
      auto ip = rts.prefix_search(default_spacename, cv[0]);
      if (ip) std::copy((*ip).first, (*ip).second, std::ostream_iterator<database::space::symbol>(std::cout, "\n"));
      else std::cout << "not found" << std::endl;
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

