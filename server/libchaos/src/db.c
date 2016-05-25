/* chaosircd - pi-networks irc server
 *
 * Copyright (C) 2003-2005  Roman Senn <smoli@paranoya.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 *
 * $Id: db.c,v 1.9 2005/01/17 19:09:50 smoli Exp $
 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/db.h>

#define DB_TMPBUF_SIZE 128 * 1024

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int           db_log;
struct sheap  db_heap;
struct sheap  db_result_heap;
struct dheap  db_dheap;
struct list   db_list;
uint32_t      db_serial;
struct db    *db_current;
char         *db_tmpbuf;

/* -------------------------------------------------------------------------- *
 * Strip whitespace                                                           *
 * -------------------------------------------------------------------------- */
static inline char *db_trim(char *s) 
{  
  uint32_t i;
  uint32_t len;
  
  len = strlen(s);
  
  for(i = len; i > 0; i--) 
  {
    if(!isspace(s[i - 1])) 
    {
      s[i] = '\0';
      break;
    }
  }
  
  return s;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void db_format_str(char   **pptr, size_t  *bptr,
                          size_t   n,    int      padding,
                          int      left, void    *arg)
{
  size_t len;
  char *escaped;
  int i;
  
  len = strlen(arg) + 1024;
  
  if(arg)
  {
    escaped = mem_dynamic_alloc(&db_dheap, strlen(arg) + 1024);
    
    len = db_escape_string(db_current, escaped, arg, strlen(arg));
  }
  else
  {
    escaped = "(null)";
    
    len = 6;
  }
  
  for(i = 0; i < len; i++)
  {
    if(*bptr >= n)
      break;
    
    *(*pptr)++ = escaped[i];
    (*bptr)++;
  }
 
  if(arg)
    mem_dynamic_free(&db_dheap, escaped);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifdef HAVE_PGSQL
static void db_notice_handler(void *arg, const char *message) 
{
  log(db_log, L_verbose, "Notice from PostgreSQL: %s", message);
}
#endif /* HAVE_PGSQL */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#if (defined HAVE_PGSQL) || (defined HAVE_MYSQL)
static struct db_result *db_new_result(struct db *db, uint32_t fields, uint64_t rows)
{
  struct db_result *result;
  
  result = mem_static_alloc(&db_result_heap);
  
  result->db = db;
  result->res.common = NULL;
  result->row = 0LLU;
  result->rows = rows;
  result->fields = fields;
  result->data = mem_dynamic_alloc(&db_dheap, (fields + 1) * sizeof(char *));
  result->fdata = mem_dynamic_alloc(&db_dheap, (fields + 1) * sizeof(char *));
  
  return result;
}
#endif
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void db_set_error(struct db *db)
{
  if(db->handle.common)
  {
    const char *error = NULL;
    
    switch(db->type)
    {
#ifdef HAVE_PQSQL
      case DB_TYPE_PGSQL:
      {
        error = PQerrorMessage(db->handle.pg);
      }
      break;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
      case DB_TYPE_MYSQL:
      {
        error = mysql_error(db->handle.my);
      }
      break;
#endif /* HAVE_MYSQL */
      default:
        strcpy(db->error, "no database support");
    }
    
    if(error)
      strlcpy(db->error, error, sizeof(db->error));
  }
  else
  {
    strcpy(db->error, "no database connection");
  }
}

/* -------------------------------------------------------------------------- *
 * Initialize DB heap                                                         *
 * -------------------------------------------------------------------------- */
void db_init(void)
{
  db_log = log_source_register("db");

  str_register('Q', db_format_str);
  
  mem_static_create(&db_heap, sizeof(struct db), DB_BLOCK_SIZE);
  mem_static_note(&db_heap, "db block heap");
  mem_static_create(&db_result_heap, sizeof(struct db_result), DB_BLOCK_SIZE * 4);
  mem_static_note(&db_result_heap, "db result heap");

  mem_dynamic_create(&db_dheap, 256 * 1024);
  
  db_serial = 0;
  
  dlink_list_zero(&db_list);

  db_tmpbuf = mem_dynamic_alloc(&db_dheap, DB_TMPBUF_SIZE);
  
  log(db_log, L_status, "Initialized [db] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy DB heap                                                            *
 * -------------------------------------------------------------------------- */
void db_shutdown(void)
{
  struct node *next;
  struct db   *db;
  
  log(db_log, L_status, "Shutting down [db] module...");
  
  dlink_foreach_safe(&db_list, db, next)
    db_destroy(db);

  mem_static_destroy(&db_result_heap);
  mem_static_destroy(&db_heap);
  mem_dynamic_destroy(&db_dheap);

  str_unregister('Q');
  
  log_source_unregister(db_log);
}  

/* -------------------------------------------------------------------------- *
 * Destroy DB instance                                                        *
 * -------------------------------------------------------------------------- */
void db_destroy(struct db *db)
{
  dlink_delete(&db_list, &db->node);
  
  mem_static_free(&db_heap, db);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct db *db_new(int type)
{
  struct db *db;
  
  switch(type)
  {
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
      break;
#endif /* HAVE_MYSQL */
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
      break;
#endif /* HAVE_PGSQL */
    default:
      log(db_log, L_fatal, "Database type #%i not supported.", type);
      return NULL;
  }
  
  
  db = mem_static_alloc(&db_heap);
  
  db->type = type;
  db->refcount = 0;
  db->id = db_serial++;
  db->error[0] = '\0';
  db->handle.common = NULL;
  
  dlink_add_tail(&db_list, &db->node, db);
  
  db_current = db;
  
  return db;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int db_connect(struct db *db, char *host, char *user, char *pass, char *dbname)
{
  db_current = db;
  
  switch(db->type)
  {
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
    {
      db->handle.pg = PQsetdbLogin(host, NULL, NULL, NULL, dbname, user, pass);
      
      if(db->handle.pg != NULL && PQstatus(db->handle.pg) == CONNECTION_OK)
      {
        PQsetNoticeProcessor(db->handle.pg, db_notice_handler, NULL);
        return 0;
      }
    }
    break;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
    {
      db->handle.my = mysql_init(NULL);
      
      if(mysql_real_connect(db->handle.my, host, user, pass,
#if MYSQL_VERSION_ID >= 32200
                            dbname,
#endif /* MYSQL_VERSION_ID */
                            3306, NULL, 0) != NULL)

      {
#if MYSQL_VERSION_ID < 32200
        if(dbname != NULL)
        {
          if(!mysql_select_db(conn, db))
            return 0;
        }
        else
#endif /* MYSQL_VERSION_ID */
          return 0;
      }      
    }
    break;
#endif /* HAVE_MYSQL */
  }
  
  db_set_error(db);
  db_close(db);
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void db_close(struct db *db)
{
  db_current = db;
  
  if(db->handle.common)
  {
    switch(db->type)
    {
#ifdef HAVE_PGSQL
      case DB_TYPE_PGSQL:
      {
        PQfinish(db->handle.pg);
      }
      break;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
      case DB_TYPE_MYSQL:
      {
        mysql_close(db->handle.my);
      }
      break;
#endif /* HAVE_MYSQL */
    }
    
    db->handle.common = NULL;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
size_t db_escape_string(struct db *db, char *to, const char *from, size_t len)
{
  db_current = db;
  
  switch(db->type)
  {
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
      return PQescapeString(to, from, len);
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
      return (size_t)mysql_escape_string(to, from, len);
#endif /* HAVE_MYSQL */
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct db_result *db_vquery(struct db *db, const char *format, va_list args)
{
  str_vsnprintf(db_tmpbuf, DB_TMPBUF_SIZE, format, args);

  switch(db->type)
  {
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
    {
      struct db_result *result = NULL;
      PGresult         *res;
      ExecStatusType    status;
      uint64_t          rows;
      uint32_t          fields;
      char             *tuples;
      
      res = PQexec(db->handle.pg, db_tmpbuf);
      
      db_set_error(db);
      
      if(res != NULL)
        status = PQresultStatus(res);
      else
        status = (ExecStatusType)PQstatus(db->handle.pg);
      
      switch(status)
      {
        case PGRES_EMPTY_QUERY:
        {
          if(res != NULL)
            PQclear(res);
          
          res = NULL;
        }
        break;
        case PGRES_BAD_RESPONSE:
        case PGRES_NONFATAL_ERROR:
        case PGRES_FATAL_ERROR:
        {
          if(res != NULL)
            PQclear(res);
          
          res = NULL;
          
          return NULL;
        }
        case PGRES_COMMAND_OK:
        default:
        {
          if(res == NULL)
            return NULL;
        }
        break;
      }
      
      rows = PQntuples(res);
      fields = PQnfields(res);
      
      tuples = PQcmdTuples(res);
      
      if(tuples && tuples[0])
        db->affected_rows = strtoull(tuples, NULL, 10);
      else
        db->affected_rows = 0LLU;
      
      if(rows || fields)
      {
        result = db_new_result(db, fields, rows);
        result->res.pg = res;
      }
      else
      {
        PQclear(res);
      }
      
      return result;
    }
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
    {
      MYSQL_RES *res;
      int ret;
      struct db_result *result = NULL;
      
      ret = mysql_query(db->handle.my, db_tmpbuf);
      
      db_set_error(db);
      
      if(ret)
        return NULL;

      db->affected_rows = mysql_affected_rows(db->handle.my);
      
      res = mysql_store_result(db->handle.my);
      
      if(res)
      {
        result = db_new_result(db, mysql_num_fields(res), mysql_num_rows(res));
      
        result->res.my = res;
        result->row = 0;
      }
        
      return result;
    }
#endif /* HAVE_MYSQL */
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct db_result *db_query(struct db *db, const char *format, ...) 
{      
  struct db_result *ret;
  
  va_list args;
  
  va_start(args, format);
  
  ret = db_vquery(db, format, args);
  
  va_end(args);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint64_t db_affected_rows(struct db *db)
{
  return db->affected_rows;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void db_free_result(struct db_result *result)
{
  switch(result->db->type)
  {
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
    {
      if(result->res.pg)
        PQclear(result->res.pg);
    }
    break;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
    {
      if(result->res.my)
        mysql_free_result(result->res.my);
    }
    break;
#endif /* HAVE_MYSQL */
  }
  
  result->res.common = NULL;
  
  mem_dynamic_free(&db_dheap, result->data);
  
  mem_static_free(&db_result_heap, result);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char **db_fetch_row(struct db_result *result)
{
  if(result->row >= result->rows)
    return NULL;
  
  switch(result->db->type)
  {
#ifdef HAVE_PGSQL
    case DB_TYPE_PGSQL:
    {
      int i;
      
      for(i = 0; i < result->fields; i++)
      {
        if(PQgetisnull(result->res.pg, result->row, i))
          result->data[i] = NULL;
        else
          result->data[i] = db_trim(PQgetvalue(result->res.pg, result->row, i));
      }
      
      result->data[i] = NULL;
      result->row++;
      
      return result->data;
    }
    break;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    case DB_TYPE_MYSQL:
    {
      char **row;
      int i;
      
      row = mysql_fetch_row(result->res.my);
      
      for(i = 0; i < result->fields; i++)
        result->data[i] = row[i];

      result->data[i] = NULL;
      
      result->row++;
      
      return result->data;
    }
    break;
#endif /* HAVE_MYSQL */
  }

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint64_t db_num_rows(struct db_result *result)
{
  return result->rows;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint32_t db_num_fields(struct db_result *result)
{
  return result->fields;
}
