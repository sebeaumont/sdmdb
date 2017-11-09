// implementation of sdmlib c interface library
#include <errno.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>

#include "sdmlib.h"
#include "database.hpp"

using namespace molemind::sdm;

const status_t sdm_database(const char* filename,
                            size_t size,
                            size_t maxsize,
                            database_t* db) {
  try {
    *db = new database::database(size, maxsize, std::string(filename));
    return AOK;
  } catch (const std::exception& e) {
    fprintf(stderr, "RUNTIME EXCEPTION: %s\n", e.what());
    return ERUNTIME;
  }
}

const status_t sdm_database_close(const database_t db) {
  delete static_cast<database::database*>(db);
  return AOK;
}

const status_t sdm_database_get_space(const database_t db,
                                      const char* spacename,
                                      space_t* space) {
  database::space* sp =
    static_cast<database::database*>(db)->get_space_by_name(std::string(spacename));
  if (sp == nullptr) return ESPACE;
  *space = sp;
  return AOK;
}

const status_t sdm_database_ensure_space(const database_t db,
                                         const char* spacename,
                                         space_t* space) {
  try {
    database::space* sp = static_cast<database::database*>(db)->
      ensure_space_by_name(std::string(spacename));
    if (sp == nullptr) return ERUNTIME; //?
    *space = sp;
    return AOK;
  } catch (const std::bad_alloc& e) {
    return EMEMORY;
  } catch (...) {
    return (const status_t) -errno;
  }
}


const status_t sdm_database_ensure_space_symbol(const database_t db,
                                                const char* spacename,
                                                const char* symbolname,
                                                symbol_t* sym) {
  auto dp = static_cast<database::database*>(db);
    // may create a space
  auto space = dp->ensure_space_by_name(spacename);
  if (!space) return ERUNTIME;
  // already exists?
  auto res = space->get_symbol_by_name(symbolname);

  if (res) {
    *sym = &(*res);
    return AOLD; // existing
  }
  // else
  try {
    // use insert to create a new symbol with elemental "fingerprint"
    database::space::inserted_t p = space->insert(symbolname, dp->randomidx().shuffle());
    
    if (p.second) {
      // insert successful:
      // N.B. the returned symbol reference is to an *immutable* entry in the index
      // i.e. const database::space::symbol& s = *(p.first);
      *sym = &(*(p.first));
      return ANEW; // created
      // do we need a new error for this?
    } else return ERUNTIME; // something (p.first) in the index stopped us inserting! 
    
  } catch (boost::interprocess::bad_alloc& e) {
    // XXX: here is where we can try and grow the heap
    return EMEMORY; // 'cos we ran out of memory!
  } catch (...) {
    return (const status_t) -errno;
  }
}


//
// ensure a symbol exists in space - may therefore entail creation and
// insertion of a new symbol
// 
const status_t sdm_database_ensure_symbol(const database_t db,
                                          const space_t space,
                                          const char* symbolname,
                                          symbol_t* sym) {

  auto dp = static_cast<database::database*>(db);
  auto sp = static_cast<database::database::space*>(space);
    

  auto res = sp->get_symbol_by_name(symbolname);
  // see if symbolname already exists
  if (res) {
    *sym = &(*res);
    return AOLD; // existing
  }
  // else
  try {
    // use insert to create a new symbol with elemental "fingerprint"
    database::space::inserted_t p = sp->insert(symbolname, dp->randomidx().shuffle());
    
    if (p.second) {
      // insert successful:
      // N.B. the returned symbol reference is to an *immutable* entry in the index
      // i.e. const database::space::symbol& s = *(p.first);
      *sym = &(*(p.first));
      return ANEW; // created
      // do we need a new error for this?
    } else return ERUNTIME; // something in the index stopped us inserting! 
    
  } catch (boost::interprocess::bad_alloc& e) {
    // XXX: here is where we can try and grow the heap
    return EMEMORY; // 'cos we ran out of memory!
  } catch (const std::exception& e) {
    fprintf(stderr, "RUNTIME EXCEPTION: %s\n", e.what());
    return ERUNTIME;
  }
}

//
// retreive a vector reference from space
//
const status_t sdm_space_get_vector(const space_t space,
                                    const char* symbolname,
                                    vector_t* vec) {
  
  auto maybe_vector = static_cast<database::database::space*>(space)->
    get_vector_by_name(std::string(symbolname));
  if (!maybe_vector) return ESYMBOL;
  *vec = &(*maybe_vector); // deref boost optional
  return AOK;
}

//
// retrieve a symbol reference from space
//
const status_t sdm_space_get_symbol(const space_t space,
                                    const char* symbolname,
                                    symbol_t* sym) {
  // lost in space...
  auto sp = static_cast<database::database::space*>(space);
    
  // already exists?
  auto maybe_symbol = sp->get_symbol_by_name(symbolname);

  if (maybe_symbol) {
    *sym = &(*maybe_symbol);
    return AOLD; 
  } else {
    return ESYMBOL;
  }
}

// new && exclusive to this library...
const status_t sdm_space_get_symbol_vector(const space_t space,
                                           const symbol_t sym,
                                           vector_t* vec) {
  auto sp = static_cast<database::space*>(space);
  auto symbol = static_cast<const database::space::symbol*>(sym);
  *vec = &(sp->get_symbol_vector(symbol));
  return AOK;
}

// 
const card_t sdm_space_get_symbols(const space_t space,
                                   const char* prefix,
                                   const card_t card_ub,
                                   term_t* tp) {
  return EUNIMPLEMENTED;
}

// get matching symbols with prefix

const card_t sdm_space_serialize_symbols(const space_t space,
                                         const char* prefix,
                                         const card_t card_ub,
                                         buffer_t* tp) {
  
  auto sp = static_cast<database::space*>(space);

  database::space::term_match tm;
  // returns number of matches found
  card_t n = sp->matching(prefix, card_ub, tm);
  (void) n; // silence unused for now
  
  std::stringstream ss; 
  //typedef cereal::JSONOutputArchive::Options options;
  
  // this block to ensure archive is flushed when it goes out of scope
  {
    cereal::JSONOutputArchive archive(ss);
    // with this: get no output at all... wtf? 
    // cereal::JSONOutputArchive archive(*ss, options::Options(6,options::IndentChar::space,0));
    archive(cereal::make_nvp("termMatch", tm));
  }

  // to make sure we can share this data we allocate a copy on the
  // heap which must be freed by caller!
  std::string* data = new std::string(ss.str().data());
  *tp = data;
  return data->size(); 
}

const char* sdm_buffer_data(buffer_t tp) {
  auto ss = static_cast<std::string*>(tp);
  return ss->c_str();
}

void sdm_buffer_free(buffer_t tp) {
  auto ss = static_cast<std::string*>(tp);
  delete ss;
}



// get a neighbourhood of points

const card_t sdm_space_get_topology(const space_t s,
                                    const vectordata_t* v,
                                    const double metric_lb,
                                    const double density_ub,
                                    const card_t card_ub,
                                    point_t* t) {
  // space 
  auto sp = static_cast<database::space*>(s);
  // load query vector
  database::space::ephemeral_vector_t vec(v);
  card_t k = 0;

  auto topo = sp->neighbourhood(vec, metric_lb, density_ub, card_ub);
  // TODO speedup by passing t directly to neighbourhood fn?
  
  for (auto i = topo.begin(); i != topo.end(); ++i, ++t) {
    ++k;
    // XXX might be stable ptr as the symbol (*i) is probably stable and c_str() is a pointer not a copy
    t->symbol = i->name.c_str(); 
    t->metric = i->similarity;
    t->density = i->density;
  }
  return k; // XXX should be the actual level set cardinality (given contraints) 
}


const status_t sdm_symbol_get_basis(const symbol_t symbol,
                                    basis_t* basis) {
  // get underlying array data from symbol
  auto sp = static_cast<const database::space::symbol*>(symbol);
  std::copy(sp->basis().begin(), sp->basis().end(), basis);
  return AOK;
}


const status_t sdm_vector_load(const vector_t v,
                               vectordata_t* vdata) {
  auto vector = static_cast<database::database::space::vector*>(v);
  vector->copy_me(vdata);
  return AOK;
}


// TODO
const status_t sdm_vector_store(const vector_t v,
                                vectordata_t vdata) {
  return EUNIMPLEMENTED;
}

