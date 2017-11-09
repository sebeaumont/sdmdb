#include <stdio.h>
#include "../rtl/sdmlib.h"

int main(int argn, char** argv) {

  database_t db;
  status_t s  = sdm_database("/Users/seb/Data/ash.sdm", 1024*1024*700, 1024*1024*700, &db);

  space_t sp;
  s = sdm_database_ensure_space(db, "words", &sp);

  term_t ts;
  
  card_t n = sdm_space_serialize_symbols(sp, "Wat", 20, &ts);
  const char* buff = sdm_buffer_data(ts);
    
  printf("%lu\n%s\n", n, buff);
  sdm_buffer_free(ts);

}
