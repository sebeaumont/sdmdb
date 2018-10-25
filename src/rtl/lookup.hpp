namespace sdm {
  namespace rtl {
    
      /// !!! experimental serializable objects
      
      struct term {
        const std::string name;
        double rho;
        term(const std::string& s, double d) : name(s), rho(d) {};
        template<class A> void serialize(A& archive) {
          archive(CEREAL_NVP(name), CEREAL_NVP(rho));
        }
      };
      
      struct term_match {
        std::string prefix;
        std::size_t matches;
        std::vector<term> terms;
        term_match() : matches(0) {};
        term_match(std::size_t m) : matches(m) {};
        template<class A> void serialize(A& archive) {
          archive(CEREAL_NVP(prefix), CEREAL_NVP(matches), CEREAL_NVP(terms));
        }
      };
      
      /// term_match on prefix
      inline const std::size_t matching(const std::string prefix,
                                        const std::size_t card_ub,
                                        term_match& tm) {

        auto sl = search(prefix);
        std::size_t matches = std::distance(sl.first, sl.second);

        //term_match tm;
        //tm.terms.reserve(card_ub);
        tm.matches = matches;
        tm.prefix = prefix;
        size_t n = 0;
        
        for (auto i = sl.first; i != sl.second && n < card_ub; ++i, ++n) {
          term t(i->name(), (*vectors)[i->_id].density());
          tm.terms.push_back(t);
        }
        
        return n;
      }
  }
}
