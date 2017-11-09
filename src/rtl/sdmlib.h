#ifndef __SDMLIB_H__
#define __SDMLIB_H__

/**
 * c header for sdmlib library API
 */

#include "sdmconfig.h"
#include "sdmstatus.h"

/*
 caveat: these pointer types are opaque to the capi so as not to
 provide a dependency on c++ classes in the implmentation:

 N.B. they are not currently validated by the library adapter
 except via static_cast on input.
*/

typedef void* database_t;
typedef void* space_t;
typedef void* vector_t;
typedef void const * symbol_t;
typedef void* term_t;

/* concrete types that are marshalled into caller space
   serialiser architecture */

typedef struct {
  const char* symbol;
  double metric;
  double density;
} point_t;

typedef void* buffer_t;

typedef SDM_VECTOR_ELEMENT_TYPE vectordata_t;
typedef size_t basis_t; 
typedef size_t card_t;


/* Functions in the API */

#ifdef __cplusplus
extern "C" {
#endif
  /**
   * ... -> database_t 
   */
  const status_t sdm_database(const char* filename,
                              size_t size,
                              size_t maxsize,
                              database_t*);

  /**
   * database_t -> ...
   */
  
  const status_t sdm_database_close(const database_t db);

  const status_t sdm_database_get_space(const database_t,
                                        const char* spacename,
                                        space_t*);

  const status_t sdm_database_ensure_space(const database_t,
                                           const char* spacename,
                                           space_t*);
  
  /* convenience fns which may afford some internal optimizations for
     interpreters and similar clients however these always incur a
     spacename lookup which may be trivial: certainly O(1) but will entail
     space creation and symbol creation and insertion at first
     occurence, high performance applications are encouraged to use
     more direct api's below. */
  
  const status_t sdm_database_ensure_space_symbol(const database_t,
                                                  const char* spacename,
                                                  const char* symbolname,
                                                  symbol_t* symbol);

  const status_t sdm_database_ensure_symbol(const database_t,
                                            const space_t,
                                            const char* symbolname,
                                            symbol_t* symbol);
  

  /**
   * space_t -> ...
   */
  
  const status_t sdm_space_get_vector(const space_t,
                                      const char* vectorname,
                                      vector_t*);
  
  const status_t sdm_space_get_symbol(const space_t,
                                      const char* symbolname,
                                      symbol_t*); 

  const status_t sdm_space_get_symbol_vector(const space_t space,
                                             const symbol_t sym,
                                             vector_t* vec);
 
  const card_t sdm_space_get_symbols(const space_t,
                                     const char* prefix,
                                     const card_t card_ub,
                                     term_t* terms);

  const card_t sdm_space_serialize_symbols(const space_t space,
                                           const char* prefix,
                                           const card_t card_ub,
                                           buffer_t* tp);
  
  const card_t sdm_space_get_topology(const space_t,
                                      const vectordata_t*,
                                      const double,
                                      const double,
                                      const card_t,
                                      point_t*);

  /**
   * symbol_t ->
   */

  const status_t sdm_symbol_get_basis(const symbol_t symbol,
                                      basis_t* basis);
  

  /**
   * vector_t -> ...
   */

  const status_t sdm_vector_load(const vector_t,
                                 vectordata_t* vdata);
  
  const status_t sdm_vector_store(const vector_t,
                                  vectordata_t vdata);

  /**
   * buffer_t -> ...
   */

  const char* sdm_buffer_data(buffer_t tp);
  
  void sdm_buffer_free(buffer_t tp);
  
 

#ifdef __cplusplus
}
#endif
#endif /* __SDMLIB_H__ */
