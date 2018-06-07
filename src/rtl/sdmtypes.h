#ifndef __SDMTYPES_H__
#define __SDMTYPES_H__

#include "sdmconfig.h"

/*
** SDM C Data types
*/

typedef SDM_VECTOR_ELEMENT_TYPE sdm_vector_t[SDM_VECTOR_ELEMS];

typedef sdm_vector_t sdm_geometry_t [];    

typedef enum {Similarity, Overlap} metric_t;


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


#ifdef __cplusplus
#define BOOL bool
#else
#define BOOL unsigned
#endif

inline BOOL sdm_error(sdm_status_t s) { return (s<0); }

#endif /* __SDMTYPES_H__ */
