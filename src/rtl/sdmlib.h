#ifndef __SDMLIB_H__
#define __SDMLIB_H__

/**
 * c header for sdmlib library API
 */

#include "sdmconfig.h"
#include "sdmtypes.h"

/* Functions in the API */

#ifdef __cplusplus
extern "C" {
#endif
  
  const sdm_status_t
  sdm_database(const sdm_name_t filename,
               sdm_size_t size,
               sdm_size_t maxsize,
               database_t*);
  
  
  const sdm_status_t
  sdm_database_close(const database_t db);


  const sdm_status_t
  sdm_namedvector(const database_t,
                  const sdm_name_t space_name,
                  const sdm_name_t symbol_name,
                  const sdm_prob_t);
  

  const sdm_status_t
  sdm_superpose(const database_t,
                const sdm_name_t target_space_name,
                const sdm_name_t target_symbol_name,
                const sdm_name_t source_space_name,
                const sdm_name_t source_symbol_name,
                const int shift);

  const sdm_status_t
  sdm_subtract(const database_t,
               const sdm_name_t target_space_name,
               const sdm_name_t target_symbol_name,
               const sdm_name_t source_space_name,
               const sdm_name_t source_symbol_name,
               const int shift);


  const sdm_status_t
  sdm_load_vector(const database_t,
                  const sdm_name_t space_name,
                  const sdm_name_t symbol_name,
                  sdm_vector_t vector);
  
  const sdm_status_t
  sdm_load_elemental(const database_t,
                     const sdm_name_t space_name,
                     const sdm_name_t symbol_name,
                     sdm_sparse_t bits);


  const sdm_status_t
  sdm_get_topology(const database_t,
                   const sdm_name_t target_space,
                   const sdm_vector_t vector,
                   const sdm_size_t cub,
                   const sdm_metric_t metric,
                   const double dlb,
                   const double dub,
                   const double mlb,
                   const double mub,
                   sdm_topology_t top);

  const sdm_status_t
  sdm_get_geometry(const database_t,
                   const sdm_name_t spacename,
                   const sdm_size_t card,
                   sdm_geometry_t g);

  /* 

  TODO: term_t or sdm_term_list_t 

  const sdm_status_t
  sdm_prefix_search(const database_t,
                    const sdm_name_t space_name,
                    const sdm_name_t prefix,
                    const sdm_size_t card_ub,
                    term_t* terms);
  */
  
  const sdm_status_t
  sdm_get_cardinality(const database_t,
                      const sdm_name_t space_name,
                      sdm_size_t* card);
  
 

#ifdef __cplusplus
}
#endif
#endif /* __SDMLIB_H__ */
