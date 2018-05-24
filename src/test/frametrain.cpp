/***************************************************************************
 * frametrain - simple online training via. stdin
 *
 * Copyright (c) Simon Beaumont 2012-2016 - All Rights Reserved.
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

// we are really testing

#include "../rtl/database.hpp"

// and timing

using namespace std;

// wall clock timer

class timer {
  
public:
  
  timer(const string& name) : name(name), start(clock_t::local_time()) {}
  
  const size_t get_elapsed_micros() {
    
    time_t now(clock_t::local_time());
    elapsed_t d = now - start;
    return d.total_microseconds();
  }
  
  friend ostream& operator<<(ostream& os, timer& t) {
    os << "[" << t.get_elapsed_micros() << "] ";
    return os;
  }
  
private:
  
  typedef boost::posix_time::ptime time_t;
  typedef boost::date_time::microsec_clock<time_t> clock_t;
  typedef boost::posix_time::time_duration elapsed_t;
  
  string name;
  time_t start;
};


// tokenzier for tsv input line
boost::char_separator<char> sep("\t");

void tokenize_line(const string& s, vector<string>& o) {

  boost::tokenizer<boost::char_separator<char>> tokens(s, sep);

  for(auto j = tokens.begin(); j != tokens.end(); ++j) {
    string t(*j);
    boost::trim(t);
    o.push_back(t);
  }
}

#define B2MB(b_) ((double)(b_)/(1024*1024))


////////////////////////////////
// entry point and command line

int main(int argc, const char** argv) {
  
  
  namespace po = boost::program_options;
  using namespace sdm;
  
  // announce
  string banner = "SDM frametrainer - Copyright (c) 2012-2018 Simon Beaumont - All Rights Reserved.\nSee: LICENCE for terms and conditions.";

  //////////////////////////////
  // global training parameters

  // command line options
  bool symmetric = false; // TODO -- what was the intension here?
  
  // create a reverse index of frames?
  bool reverse_index = false;
  // co train terms in termspace?
  bool cotrain = false;
  // train with differentiated term instances
  bool diffterms = false;
  
  string framespace;
  string termspace;
  
  size_t initial_size;
  size_t maximum_size;
  
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("image", -1);
  
  desc.add_options()
    ("help", "SDM frametrainer -- read (tsv) frames from stdin")
    ("multisense", po::bool_switch(&diffterms),
     "train with differentiated instances of terms")
    ("cotrain", po::bool_switch(&cotrain),
     "co-train terms in termspace")
    ("termspace", po::value<string>(),
     "name of space for terms")
    ("framespace", po::value<string>(),
     "name of space for frames")
    ("size", po::value<size_t>(&initial_size)->default_value(700),
     "initial size of heap in MB")
    ("maxsize", po::value<size_t>(&maximum_size)->default_value(700),
     "maximum size of heap in MB")
    ("image", po::value<string>(),
     "heap image name (must be a valid path)");
  
  po::variables_map opts;
  
  // XXX TODO try...catch around this! exception is part of expected flow!
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), opts);
  po::notify(opts);
  
  if (opts.count("help")) {
    cout << banner << endl;
    cout << desc <<  endl;
    return 1;
  }

  // must have a heap mmf
  if (!opts.count("image")) {
    cout << "Heap image is a required parameter!" << endl;
    cout << desc <<  endl;
    return 3;
  }

  if (!opts.count("termspace")) {
    cout << "Termspace is a required parameter!" << endl;
    cout << desc <<  endl;
    return 5;
  }

  termspace = opts["termspace"].as<string>();
  
  if (opts.count("framespace")) {
    framespace = opts["framespace"].as<string>();
    reverse_index = true;
  }


  string heapfile(opts["image"].as<string>());

  // warn user maybe they just want to parse the input...
  if (!reverse_index && !cotrain) {
    cout << "Warning: not co-training terms and no framespace given so no training will take place!" << endl;
  }
  cout << "============================================="         << endl;
  cout << banner                                                  << endl;
  cout << "---------------------------------------------"         << endl;
  cout << "database:   " << heapfile                              << endl;
  cout << "size:       " << initial_size                          << endl;
  cout << "maxsize:    " << maximum_size                          << endl;
  cout << "termspace:  " << termspace                             << endl;
  cout << "framespace: " << (reverse_index ? framespace : "None") << endl;
  cout << "cotrain:    " << cotrain                               << endl;
  cout << "multisense: " << diffterms                             << endl;
  cout << "============================================="         << endl;

  // create database with requirement
  database db(initial_size * 1024 * 1024, maximum_size * 1024 * 1024, heapfile);
  
  // print out all the existing spaces and cardinalities
  vector<string> spaces = db.get_named_spaces();
  
  if (spaces.size() > 0) {
    cout << "existing spaces in image:" << endl;
    // see if we can find space names
    std::vector<std::string> spaces = db.get_named_spaces();
    
    for (auto sn: spaces) {
      auto ret = db.get_space_cardinality(sn);
      std::cout << sn << " #" << ret.second << std::endl;
    }
  }
  
  cout << "reading data frames from stdin..." << endl;
  // accumualated stats...
  u_int frames = 0;

  u_int empty = 0;
  u_int rows = 0;
  
  // main i/o loop
  string input;
  // frameid for reverse index
  string frameid;  

  // simple cline processor
  while (getline(cin, input)) {
    rows++;
    
    boost::trim(input);
    vector<string> tv;
  
    tokenize_line(input, tv);

    // must at least have a frame id and a term
    if (tv.size() > 1) {
      
      // change of frame 
      if (frameid != tv[0]) {
        frameid = tv[0];
        frames++;
      }
      
      // we need to permute the pairs treating this as a partial order
      // currently the frame is "dimensionless" and relative position unobserved
      
      // un-ordered set of terms in frame
      // xxx should this be a set now in light of multisense capability?
      //     e.g. repeated terms in given frame may have different senses!
      //set<string> termset(tv.begin()+1, tv.end());
      list<string> termset(tv.begin()+1, tv.end());

      // assert reverse index if required
      if (reverse_index) for (string term: termset) {
          db.superpose(framespace, frameid, termspace, term, diffterms);
      }

      // cotrain terms in termspace
      if (cotrain) {
        // co-train pairwise combinatations of terms
        for (auto first = termset.begin(); first != termset.end(); ++first) {
          for (auto next = std::next(first); next != termset.end(); ++next) {
            // assert: first R next
            db.superpose(termspace, *first, termspace, *next, diffterms);
            // if aRb => bRa then reify, latch current sense
            if (symmetric) db.superpose(termspace, *next, termspace, *first);
          }
        }
      }
      
    } else empty++; // empty row
  }
 
  // goodbye from me and goodbye from him...
  cout << "at end of input rows: " << rows
       << " frames: " << frames
       << " empty frames: " << empty << endl;
  
  if (cotrain) cout << termspace
                    << " #"
                    << db.get_space_cardinality(termspace).second << endl;

  if (reverse_index) cout << framespace
                          << " #"
                          << db.get_space_cardinality(framespace).second << endl;
  
  cout << (db.check_heap_sanity() ? ":-)" : ":-(") << " free: " << B2MB(db.free_heap()) << endl;
  
  return 0;
}

