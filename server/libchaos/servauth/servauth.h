/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003  Roman Senn <smoli@paranoya.ch>
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
 * $Id: servauth.h,v 1.5 2004/01/23 07:46:28 smoli Exp $
 */

#ifndef SERVAUTH_SERVAUTH_H
#define SERVAUTH_SERVAUTH_H

#define MAX_QUERIES 256

/* timeouts in miliseconds */
#define AUTH_TIMEOUT 10000
#define DNS_TIMEOUT  20000

#define CACHE_AUTH_SIZE  512
#define CACHE_DNS_SIZE   256
#define CACHE_PROXY_SIZE 256

#include "control.h"

extern struct servauth_query servauth_queries[MAX_QUERIES];
extern control_t             servauth_control;
extern struct cache_auth     servauth_authcache;
extern struct cache_dns      servauth_dnscache;
extern struct cache_proxy    servauth_proxycache;

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
extern void servauth_shutdown(void); 

#endif /* SERVAUTH_SERVAUTH_H */
