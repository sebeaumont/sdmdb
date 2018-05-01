/* Copyright (C) Simon Beaumont 2015-2016 - All Rights Reserved */
#pragma once

#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/optional.hpp>

// symbol type
#include "symbol.hpp"


#ifdef HAVE_DISPATCH
#include <dispatch/dispatch.h>
#else
// openMP
#endif


/////////////////////

namespace sdm {
  
  namespace mms {
    
    using boost::multi_index_container;
    using namespace boost::multi_index;
    namespace bip = boost::interprocess;

    /** 
     * this class template can be instantiated in runtime library
     * source or inlined in application code; the important
     * implementation details are the types and sizes of the vectors
     * of the underlying vector space and the sparsity of the random
     * (a.k.a. elemental) vectors
     */

    template <typename VectorElementType, std::size_t VectorElems, std::size_t ElementalBits, class SegmentClass>

    // for a symbol_space
    
    class symbol_space final {

      // stored objects have to use this segment type for heap allocators
      typedef SegmentClass segment_t;
      
      // allocators derived from segment type
      typedef typename segment_t::segment_manager segment_manager_t;
      
      typedef bip::basic_string<char,std::char_traits<char>, bip::allocator<char, segment_manager_t>> shared_string_t;
      typedef typename bip::allocator<void, segment_manager_t> void_allocator_t;
      
      ////////////////////////
      // implement symbol type
      
      typedef symbol<segment_manager_t, shared_string_t, void_allocator_t> symbol_t;
      typedef typename symbol_t::elemental_vector_t basis_t;
      typedef typename symbol_t::semantic_vector_t vector_t;
      
      // allocator for symbol
      
      typedef bip::allocator<symbol_t, segment_manager_t> symbol_allocator_t;
      
      
      // shared string helpers
      
      inline shared_string_t shared_string(const std::string& s) {
        return shared_string_t(s.c_str(), allocator);
      }

      inline shared_string_t shared_string(const char* s) {
        return shared_string_t(s, allocator); 
      }

      // partial (prefix) string comparison
      
      struct partial_string {
        partial_string(const shared_string_t& str) : str(str) {}
        shared_string_t str;
      };
      
      struct partial_string_comparator {
        bool operator()(const shared_string_t& x, const shared_string_t& y) const {
          return x<y;
        }

        bool operator()(const shared_string_t& x,const partial_string& y) const {
          return x.substr(0,y.str.size())<y.str;
        }

        bool operator()(const partial_string& x,const shared_string_t& y) const {
          return x.str<y.substr(0,x.str.size());
        }
      };


      ////////////////////////////////////////////////////////////////////
      // shared memory mapped multi index container type with it's indexes
      
      typedef multi_index_container<
        symbol_t,
        indexed_by<
          hashed_unique<BOOST_MULTI_INDEX_MEMBER(symbol_t, shared_string_t, _name)>,
          ordered_unique<BOOST_MULTI_INDEX_MEMBER(symbol_t, shared_string_t, _name),
                         partial_string_comparator>,
          random_access<> // xxx
          >, symbol_allocator_t
        > symbol_table_t;


      
      
      ///////////////////////////////////
      // symbol_space public interface //
      ///////////////////////////////////
      
    public:

      // constructor create segment for space
      
      symbol_space(const std::string& s, segment_t& m)
        : name(s), segment(m), allocator(segment.get_segment_manager()) {
        index = segment.template find_or_construct<symbol_table_t>(name.c_str())(allocator);
     }

      
      // delete the rest of the gang don't ever want to copy a space -- but move?

      symbol_space(const symbol_space&) = delete;
      symbol_space(symbol_space&&) = delete;
      const symbol_space& operator=(const symbol_space&) = delete;
      const symbol_space& operator=(symbol_space&&) = delete;

      
      // printer symbol_space stats
      
      friend std::ostream& operator<<(std::ostream& os, symbol_space& t) {
        os << t.spacename() << " #" << t.entries(); 
        return os;
      }  
     

      typedef symbol_t symbol; // public face of symbol
      typedef vector_t vector; // public face of vector
      
      /////////////////////////////////////
      /// multi index container indexes ///
      /////////////////////////////////////
      
      typedef std::pair<typename symbol_table_t::iterator, bool> inserted_t;
      
      /// attempt to insert symbol into space
      
      inline inserted_t insert(const std::string& k, const std::vector<unsigned>& fp) {
        return index->insert(symbol(k.c_str(), fp, allocator));
      }
      
      /// random access index
      
      typedef typename symbol_table_t::template nth_index<2>::type symbol_by_index;
      
      /// overload [] and delegate to direct index
      
      inline const symbol& operator[](std::size_t i) {
        symbol_by_index& symbols = index->template get<2>(); 
        return symbols[i]; 
      }

      /// !!! expose vectors directly
      /*
      inline boost::optional<vector&> get_vector_by_name(const std::string& k) {
        symbol_by_name& name_idx = index->template get<0>();
        typename symbol_by_name::iterator it = name_idx.find(shared_string(k));
        if (it == name_idx.end()) return boost::none;
        // I do know what I am doing here honest! XXX
        else {
          auto v = it->vector();
          return const_cast<vector&>(v);
        }
      }
      */
      
      ////////////////////////////
      /// lookup symbol by name //
      ////////////////////////////
      
      typedef typename symbol_table_t::template nth_index<0>::type symbol_by_name;

      inline boost::optional<const symbol&> get_symbol_by_name(const std::string& k, bool refcount=false) {
        symbol_by_name& name_idx = index->template get<0>();
        typename symbol_by_name::iterator i = name_idx.find(shared_string(k));
        if (i == name_idx.end()) return boost::none;
        else if (refcount) {
          index->modify(i, bump_reference());
        } 
        return *i;
      }

      /// functor to bump reference count
      struct bump_reference {
        bump_reference(void) {
        }
        void operator() (symbol& s) {
          s._instance++;
        }
      };
        
      /* CAUTION: can allow callers to side-effect symbol state -- must not alter index state or memory layout */ 
      inline boost::optional<symbol&> get_non_const_symbol_by_name(const std::string& k, bool refcount=false) {
        symbol_by_name& name_idx = index->template get<0>();
        typename symbol_by_name::iterator i = name_idx.find(shared_string(k));
        if (i == name_idx.end()) return boost::none;
        symbol& s = const_cast<symbol&>(*i);
        return s;
      }

      ///////////////////////////////////////
      /// lookup symbols by prefix of name //
      ///////////////////////////////////////
      
      typedef typename symbol_table_t::template nth_index<1>::type symbol_by_prefix;  
      typedef typename symbol_by_prefix::iterator symbol_iterator;

      inline std::pair<symbol_iterator, symbol_iterator> search(const std::string& k) {
        symbol_by_prefix& name_idx = index->template get<1>();
        return name_idx.equal_range(partial_string(shared_string(k)));
      }

            
      /// delegated space iterators

      typedef typename symbol_table_t::iterator iterator;
      typedef typename symbol_table_t::const_iterator const_iterator;

      //inline iterator begin() { return index->begin(); }
      //inline iterator end() { return index->end(); }

      inline const_iterator begin() { return index->begin(); }
      inline const_iterator end() { return index->end(); }

      // delegated properties
      inline const size_t entries() { return index->size(); }
      inline const std::string spacename() const { return name; }

      
    private:    

      std::string          name; 
      symbol_table_t*      index;
      segment_t&           segment;
      void_allocator_t     allocator;
    };
  }
}
