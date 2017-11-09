#ifndef __SDMSTATUS_H__
#define __SDMSTATUS_H__

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
  EUNIMPLEMENTED = -32

};  

typedef enum sdm_status status_t;

/* check what c-language will accept here is bool ansi c? */
/* int sdm_error(status_t s) { return (s<0); } */

#endif
