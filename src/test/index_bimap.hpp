#pragma once
#include <map>
#include <iostream>


/////////////////////////////////////
// custom bimap with size_t indexes 

template <typename L>
class index_bimap {
  std::map<L, std::size_t> _left;
  std::map<std::size_t, L> _right;
  
public:
  
  inline void insert(const L& l, const std::size_t& r) {
    _left[l] = r;
    _right[r] = l;
  }
  
  inline const std::size_t left(const L& l) const { return _left.at(l); }
  
  inline const L right(const std::size_t& r) const { return _right.at(r); }
  
  inline const std::size_t ensure(const L& l) {
    try {
      return _left.at(l);
    } catch (std::out_of_range& ex) {
      std::size_t i = _left.size();
      _left[l] = i;
      _right[i] = l;
      return i;
    }
  }

  
  inline const std::size_t size() { return _left.size(); }
  
  // serialize
  
  inline const bool serialize(std::ostream& outs=std::cout) {
    // write out L's in ascending index order
    if (outs.good()) { 
      for (auto const& entry: _right) {
        outs << entry.first << "\t" << entry.second << "\n"; // avoid endl flush!
      }
      return true;
    } else return false;
  }


  // de-serialize
  
  inline const bool deserialize(std::istream& ins=std::cin) {
    if (ins.good()) {

      std::size_t idx;
      L feature;
      
      _left.clear();
      _right.clear();
      
      while (ins >> idx >> feature)
        insert(feature, idx); 
      return true;
      
    } else return false;
  }
};

