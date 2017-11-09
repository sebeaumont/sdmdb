#ifndef __SDMCONFIG_H__
#define __SDMCONFIG_H__
/* 
 * Copyright (c) 2012-2017 Simon Beaumont
 * This is the fundamental config file for compile time (baked in)
 * SDM implementation constants. DO NOT EXCEED THESE BOUNDS.
 *
 * TODO: buid config of these vars
 */

#define SDM_VECTOR_ELEMENT_TYPE unsigned long long 
#define SDM_VECTOR_ELEMS 256
#define SDM_VECTOR_BASIS_SIZE 16

#define SDM_VECTOR_PAYLOAD_SIZE sizeof(SDM_VECTOR_ELEMENT_TYPE)*SDM_VECTOR_ELEMS

/* #include <stdlib.h> language-c cant grok modern c!  so we are forced
   into this hack as we would normally take platform defaults from stdlib. */
#ifndef size_t
typedef unsigned long size_t;
#endif
/* There may be more... */
#endif
