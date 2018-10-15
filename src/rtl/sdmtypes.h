#ifndef __SDMTYPES_H__
#define __SDMTYPES_H__

#include "sdmconfig.h"

/*
** SDMLIB API types
*/


/* C++ */

#ifdef __cplusplus
#define BOOL bool
typedef std::size_t sdm_size_t;

/* ANSI C */
#else
#define BOOL unsigned

/* #include <stdlib.h> language-c cant grok modern c! so we are forced
   into this hack as we would normally take platform defaults from stdlib.
   TODO: we might be able to configure a type at build time */

#ifndef size_t
typedef unsigned long size_t;
#endif

typedef size_t sdm_size_t;
#endif


/* C friendly API types */

typedef void* database_t;

typedef char* sdm_name_t;

typedef SDM_VECTOR_ELEMENT_TYPE sdm_vector_t[SDM_VECTOR_ELEMS];

typedef struct sdm_point { const char* name; double density; unsigned refcount; } sdm_point_t;

typedef sdm_point_t sdm_geometry_t [];    

typedef unsigned sdm_sparse_t [];

typedef float sdm_prob_t; /* [0,1] */

/* UC */

typedef struct sdm_neighbour {sdm_point_t p; double metric; } sdm_neighbour_t;

typedef sdm_neighbour_t sdm_topology_t[];

enum sdm_metric {similarity, overlap};

typedef enum sdm_metric sdm_metric_t;


enum sdm_status {
  SZERO = 0,
  /* sucess sometimes with info  */

  AOK = 0,
  ANEW = 1,
  AOLD = 2,

  /* errors */
  
  ESPACE = -2,
  ESYMBOL = -4,
  EMEMORY = -8,
  ERUNTIME = -16, 
  EUNIMPLEMENTED = -32,
  EINDEX = -64

};  

typedef enum sdm_status sdm_status_t;


inline BOOL sdm_error(sdm_status_t s) { return (s<0); }

#endif /* __SDMTYPES_H__ */
