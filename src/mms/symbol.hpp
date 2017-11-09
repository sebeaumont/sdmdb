#pragma once
#include "elemental_vector.hpp"

namespace molemind { namespace sdm { namespace mms {
  
  /////////////////////////////////////////////////////////////////////
  // symbol - named vector with lazily computed elemental fingerprint
  //

  template <typename SegmentManager, typename StringType, typename Allocator, std::size_t ElementalBits>
  
  struct symbol final {
  
    // sparse stored (immutable) fingerprint
    typedef elemental_vector<std::size_t, SegmentManager> elemental_vector_t;
    typedef StringType shared_string_t;
    typedef Allocator void_allocator_t;
  
    // see if this is useful... updating the node could be expensive.
    //enum state_t { NEW, USED, OLD, FREE };
    std::size_t _id;
    shared_string_t _name;
  
  private:
    elemental_vector_t _basis;
  
  public:
  
    // constructor with fingerprint
    symbol(const char* s,
           const std::size_t i,
           const std::vector<size_t>& fp,
           const void_allocator_t& void_alloc)
      : _id(i),
        _name(s, void_alloc),
        _basis(fp, ElementalBits, void_alloc) {}
    
    // constructor without fingerprint
    /*
    symbol(const char* s,
           const void_allocator_t& void_alloc)
      : _state(NEW),
        _name(s, void_alloc),
        _basis(ElementalBits, void_alloc) {}
    */
  
    inline const std::string name(void) const {
      return std::string(_name.begin(), _name.end());
    }
  
  
    typedef elemental_vector_t basis_vector_t;
  
    inline const basis_vector_t& basis(void) const {
      return _basis;
    }
  
    // printer
    friend std::ostream& operator<<(std::ostream& os, const symbol& s) {
      os << s._name;
      return os;
    }
  
  };
}}}
