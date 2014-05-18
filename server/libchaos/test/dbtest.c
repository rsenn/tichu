#include <libchaos/io.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/db.h>

int dbtest_type = DB_TYPE_MYSQL;
char *dbtest_host = "localhost";
char *dbtest_user = "visualize";
char *dbtest_pass = "***";
char *dbtest_dbname = "visualize";
/*int dbtest_type = DB_TYPE_PGSQL;
char *dbtest_host = "localhost";
char *dbtest_user = "enki";
char *dbtest_pass = "***";
char *dbtest_dbname = "babel";*/

int dbtest_log;

void dbtest()
{
  struct db *db;
  struct db_result *result;
  char **row;
  
  db = db_new(dbtest_type);
  
  if(!db_connect(db, dbtest_host, dbtest_user, dbtest_pass, dbtest_dbname))
    log(dbtest_log, L_status, "Database connection OK (Type = %s)", (db->type == DB_TYPE_PGSQL ? "PostgreSQL" : "MySQL"));
  
  result = db_query(db, "SELECT * FROM nodes ORDER BY id");

  while((row = db_fetch_row(result)))
  {
    log(dbtest_log, L_status, " %-10s %-10s %-10s", row[0], row[1], row[2]);
  }
  
  db_free_result(result);
  
  db_close(db);
}

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  dbtest_log = log_source_register("dbtest");
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  db_init();
  
  dbtest();
  
  db_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
