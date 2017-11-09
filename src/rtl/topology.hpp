#pragma once

namespace molemind {
  namespace sdm {
  
    // this is exists for sorting point sets only

    // TODO make this a template class to avoid copying database
    // strings around
    
    struct point {
    
      std::string name;
      double similarity;
      double density;
      
      point(const std::string& v, const double s, const double d) : name(v), similarity(s), density(d) {}
      
      /// similarity is 1-d
      /// comparison for descending similarity
      bool operator< (const point& s) const {
        return similarity > s.similarity;
      }
      
      bool operator==(const point& s) const {
        return name == s.name && similarity == s.similarity;
      }
      bool operator!=(const point& s) const {
        return name != s.name || similarity != s.similarity;
    }
      
    friend std::ostream& operator<<(std::ostream& os, point& p) {
      os <<  p.name << "\t" << p.similarity << "\t" << p.density;
      return os;
    }
      
    };
    
    typedef std::vector<point> topology;
    
  }
}
