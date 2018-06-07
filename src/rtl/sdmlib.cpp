// implementation of sdmlib c interface library
#include <errno.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>

#include "sdmlib.h"
#include "database.hpp"

using namespace sdm;

/* the c rtl implementation */

const sdm_status_t
sdm_database(const char* filename,
             size_t size,
             size_t maxsize,
             database_t* db) {
  try {

    *db = new database(std::string(filename), size, maxsize);
    return AOK;

  } catch (const std::exception& e) {

    fprintf(stderr,
            "SDMLIB: %s (sdm_database %s %lu %lu)\n",
            e.what(), filename, size, maxsize);
    return ERUNTIME;
  }
}

const sdm_status_t
sdm_database_close(const database_t db) {
  delete static_cast<database*>(db);
  return AOK;
}



const sdm_status_t
sdm_load_vector(const database_t db,
                const char* space_name,
                const char* symbol_name,
                sdm_vector_t vector) {

  return static_cast<database*>(db)->load_vector(std::string(space_name),
                                                 std::string(symbol_name),
                                                 vector);
}
