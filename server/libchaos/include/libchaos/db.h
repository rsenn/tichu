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
 * $Id: db.h,v 1.7 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_DB_H
#define LIB_DB_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define DB_TYPE_PGSQL 0
#define DB_TYPE_MYSQL 1

#ifdef HAVE_MYSQL
#include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_PGSQL
#include <libpq-fe.h>
#endif /* HAVE_PGSQL */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct db_result {
  struct db  *db;
  union {
#ifdef HAVE_PGSQL
    PGresult  *pg;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    MYSQL_RES *my;
#endif /* HAVE_MYSQL */
    void      *common;
  } res;
  
  uint64_t    row;
  uint64_t    rows;
  uint32_t    fields;
  char      **data;
  char      **fdata;
};

struct db {
  struct node              node;
  uint32_t                 id;
  uint32_t                 refcount;
  int                      type;
  
  union {
#ifdef HAVE_PGSQL
    PGconn                *pg;
#endif /* HAVE_PGSQL */
#ifdef HAVE_MYSQL
    MYSQL                 *my;
#endif /* HAVE_MYSQL */
    void                  *common;
  } handle;
  uint64_t                 affected_rows;
  char                     error[256];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 db_log;
extern struct sheap        db_heap;
extern struct list         db_list;
extern uint32_t            db_serial;

/* -------------------------------------------------------------------------- *
 * Initialize DB heap                                                         *
 * -------------------------------------------------------------------------- */
extern void                db_init              (void);
  
/* -------------------------------------------------------------------------- *
 * Destroy DB heap                                                            *
 * -------------------------------------------------------------------------- */
extern void                db_shutdown          (void);
  
/* -------------------------------------------------------------------------- *
 * Destroy DB instance                                                        *
 * -------------------------------------------------------------------------- */
extern void                db_destroy           (struct db  *db);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct db          *db_new               (int         type);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 db_connect           (struct db  *db,
                                                 char       *host,
                                                 char       *user,
                                                 char       *pass,
                                                 char       *dbname);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct db_result   *db_query             (struct db  *db,
                                                 const char *format, 
                                                 ...);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void                db_close             (struct db  *db);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern size_t              db_escape_string     (struct db  *db,
                                                 char       *to,
                                                 const char *from,
                                                 size_t      len);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void                db_free_result       (struct db_result *result);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern char              **db_fetch_row         (struct db_result *result);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint64_t            db_num_rows          (struct db_result *result);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint32_t            db_num_fields        (struct db_result *result);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint64_t            db_affected_rows     (struct db *db);
  

#endif /* LIB_DB_H */
