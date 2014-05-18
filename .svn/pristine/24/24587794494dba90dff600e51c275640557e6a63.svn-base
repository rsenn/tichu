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
 * $Id: httpc.c,v 1.8 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/connect.h>
#include <libchaos/httpc.h>
#include <libchaos/timer.h>
#include <libchaos/dlink.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/config.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int              httpc_log;
struct sheap     httpc_heap;
struct sheap     httpc_vars_heap;
struct dheap     httpc_dheap;
struct list      httpc_list;
uint32_t         httpc_id;
struct protocol *httpc_proto;
char             httpc_hexchars[] = "0123456789ABCDEF";
char             httpc_version[] = PACKAGE_NAME" v"PACKAGE_VERSION" ("PACKAGE_RELEASE")";
uint8_t          httpc_hextable[256] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void httpc_callback(int fd, struct connect *connect)
{
  struct httpc *hcptr = *(struct httpc **)connect->args;
  
  if(connect->status == CONNECT_DONE)
  {
    log(httpc_log, L_status, "Sending request");
    
    hcptr->fd = connect->fd;
    io_register(hcptr->fd, IO_CB_READ, httpc_recv, hcptr);
    io_queue_control(hcptr->fd, ON, ON, ON);
    httpc_send(hcptr);
    hcptr->status = HTTPC_SENT;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static size_t httpc_parse_protocol(struct httpc *hcptr)
{
  char  *s;
  size_t i = 0;
  
  hcptr->protocol[0] = '\0';
  
  if((s = strstr(hcptr->url, "://")))
  {
    for(i = 0; isalpha(hcptr->url[i]) && i < sizeof(hcptr->protocol) - 1; i++)
      hcptr->protocol[i] = hcptr->url[i];
    
    hcptr->protocol[i] = '\0';
  }
  
  if(hcptr->protocol[0] == '\0')
    strcpy(hcptr->protocol, "http");

  if(!strcmp(hcptr->protocol, "http"))
  {
    hcptr->port = 80;
  }
  else if(!strcmp(hcptr->protocol, "https"))
  {
    hcptr->port = 443;
  }
  else
  {
    log(httpc_log, L_warning, "Invalid protocol '%s', setting to 'http'.",
        hcptr->protocol);
    strcpy(hcptr->protocol, "http");
    hcptr->port = 80;
  }
  
  return s ? s - hcptr->url + 3 : i;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static size_t httpc_parse_host(struct httpc *hcptr, size_t index)
{
  size_t i;
  
  for(i = 0; hcptr->url[index + i] &&
      i < sizeof(hcptr->host) - 1; i++)
  {
    if(hcptr->url[index + i] == ':' || hcptr->url[index + i] == '/')
      break;
    
    hcptr->host[i] = hcptr->url[index + i];
  }
  
  hcptr->host[i] = '\0';  
  
  return index + i;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static size_t httpc_parse_port(struct httpc *hcptr, size_t index)
{
  size_t i;
  char   portnum[6];
  
  if(hcptr->url[index] != ':')
  {
    for(i = 0; hcptr->url[index + i] && hcptr->url[index + i] != '/'; i++);
  }
  else
  {
    index++;
    
    for(i = 0; isdigit(hcptr->url[index + i]) &&
        i < sizeof(portnum) - 1; i++)
      portnum[i] = hcptr->url[index + i];
    
    portnum[i] = '\0';
    
    hcptr->port = strtoul(portnum, NULL, 10);
  }
  
  return index + i;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static size_t httpc_parse_location(struct httpc *hcptr, size_t index)
{
  size_t i;
  
  for(i = 0; hcptr->url[index + i] &&
      i < sizeof(hcptr->location) - 1; i++)
  {
    if(hcptr->url[index + i] == '?')
      break;
     
    hcptr->location[i] = hcptr->url[index + i];
  }
  
  if(i == 0)
    hcptr->location[i++] = '/';
    
  hcptr->location[i] = '\0';
  
  return index + i;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#if 0
static size_t httpc_parse_vars(struct httpc *hcptr, size_t index)
{
  return 0;
}
#endif
/* -------------------------------------------------------------------------- *
 * Initialize httpc heap.                                                     *
 * -------------------------------------------------------------------------- */
void httpc_init(void)
{
  httpc_log = log_source_register("httpc");
  
  dlink_list_zero(&httpc_list);

  httpc_id = 0;
  
  mem_static_create(&httpc_heap, sizeof(struct httpc), HTTPC_BLOCK_SIZE);
  mem_static_note(&httpc_heap, "httpc block heap");
  mem_static_create(&httpc_vars_heap, sizeof(struct httpc), HTTPC_BLOCK_SIZE * 2);
  mem_static_note(&httpc_vars_heap, "httpc variable heap");
  mem_dynamic_create(&httpc_dheap, HTTPC_MAX_BUF);
  mem_dynamic_note(&httpc_dheap, "httpc buffer heap");

  httpc_proto = net_register(NET_CLIENT, "http", httpc_callback);
  
  log(httpc_log, L_status, "Initialized [httpc] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy httpc heap.                                                        *
 * -------------------------------------------------------------------------- */
void httpc_shutdown(void)
{
  struct httpc *hcptr;
  struct httpc *next;
  
  log(httpc_log, L_status, "Shutting down [httpc] module...");
  
  dlink_foreach_safe(&httpc_list, hcptr, next)
    httpc_delete(hcptr);
  
  mem_dynamic_destroy(&httpc_dheap);
  mem_static_destroy(&httpc_vars_heap);
  mem_static_destroy(&httpc_heap);
  
  net_unregister(NET_CLIENT, "http");
  
  log_source_unregister(httpc_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static struct httpc_var *httpc_var_find(struct httpc *hcptr, const char *name)
{
  struct httpc_var *hvptr;
  uint32_t          hash;
  
  hash = strhash(name);
  
  dlink_foreach(&hcptr->vars, hvptr)
  {
    if(hvptr->hash == hash)
      return hvptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_var_vset(struct httpc *hcptr, const char *name, 
                    const char   *value, va_list     args)
{
  struct httpc_var *hvptr;
  char              var[64];
  
  strlcpy(var, name, sizeof(var));
  
  if((hvptr = httpc_var_find(hcptr, var)) == NULL)
  {
    hvptr = mem_static_alloc(&httpc_vars_heap);
    
    hvptr->hash = strhash(var);
    strcpy(hvptr->name, var);
    
    dlink_add_tail(&hcptr->vars, &hvptr->node, hvptr);
  }
  
  if(value)
    vsnprintf(hvptr->value, sizeof(hvptr->value), value, args);
  else
    hvptr->value[0] = '\0';
}

void httpc_var_set(struct httpc *hcptr, const char *name, 
                   const char   *value, ...)
{
  va_list args;
  
  va_start(args, value);
  httpc_var_vset(hcptr, name, value, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_var_destroy(struct httpc *hcptr)
{
  struct httpc_var *hvptr;
  
  dlink_foreach(&hcptr->vars, hvptr)
    mem_static_free(&httpc_vars_heap, hvptr);
  
  dlink_list_zero(&hcptr->vars);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_var_build(struct httpc *hcptr)
{
  struct httpc_var *hvptr;
  size_t            n;
  size_t            size;
  
  if(hcptr->content)
  {
    mem_dynamic_free(&httpc_dheap, hcptr->content);
    hcptr->content = NULL;
  }
  
  size = (hcptr->vars.size * (64 + 1024)) + 1;
  
  hcptr->content = mem_dynamic_alloc(&httpc_dheap, size);
  
  if(hcptr->content)
  {
    n = 0;
    
    dlink_foreach(&hcptr->vars, hvptr)
    {
      if(n)
        hcptr->content[n++] = '&';
      
      n += httpc_url_encode(&hcptr->content[n], hvptr->name, size - n - 1, 0);
      hcptr->content[n++] = '=';
      n += httpc_url_encode(&hcptr->content[n], hvptr->value, size - n - 1, 0);
      
      if(n + 1024 > size)
        hcptr->content = mem_dynamic_realloc(&httpc_dheap, hcptr->content, n + 1024);
    }
    
    hcptr->content_length = n;
    hcptr->content[n++] = '\0';
    
    hcptr->content = mem_dynamic_realloc(&httpc_dheap, hcptr->content, n);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc_var *httpc_header_find(struct httpc *hcptr, const char *name)
{
  struct httpc_var *hvptr;
  uint32_t          hash;
  
  hash = strhash(name);
  
  dlink_foreach(&hcptr->header, hvptr)
  {
    if(hvptr->hash == hash)
      return hvptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_header_set(struct httpc *hcptr, const char *name,
                      const char   *value)
{
  struct httpc_var *hvptr;
  char              var[64];
  
  strlcpy(var, name, sizeof(var));
  
  if((hvptr = httpc_header_find(hcptr, var)) == NULL)
  {
    hvptr = mem_static_alloc(&httpc_vars_heap);
    
    hvptr->hash = strhash(var);
    strcpy(hvptr->name, var);
    
    dlink_add_tail(&hcptr->header, &hvptr->node, hvptr);
  }

  strlcpy(hvptr->value, value, sizeof(hvptr->value));
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_header_destroy(struct httpc *hcptr)
{
  struct httpc_var *hvptr;
  
  dlink_foreach(&hcptr->header, hvptr)
    mem_static_free(&httpc_vars_heap, hvptr);
  
  dlink_list_zero(&hcptr->header);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_read(int fd, struct httpc *hcptr)
{
}

/* -------------------------------------------------------------------------- *
 * Add a httpc.                                                               *
 * -------------------------------------------------------------------------- */
struct httpc *httpc_add(const char *url)
{
  struct httpc *hcptr;
  
  hcptr = mem_static_alloc(&httpc_heap);
  
  memset(hcptr, 0, sizeof(struct httpc));
  
  hcptr->id = httpc_id++;
  strlcpy(hcptr->url, url, sizeof(hcptr->url));
  
  httpc_url_parse(hcptr);
  
  if((hcptr->connect = connect_add(hcptr->host, hcptr->port, httpc_proto,
                                   HTTPC_TIMEOUT, HTTPC_INTERVAL, 0, 
                                   hcptr->ssl, NULL)) == NULL)
  {
    mem_static_free(&httpc_heap, hcptr);
    return NULL;
  }
  
  hcptr->connect->silent = 1;
  
  httpc_set_name(hcptr, hcptr->host);
  
  connect_set_args(hcptr->connect, &hcptr, sizeof(hcptr));
  
  dlink_add_tail(&httpc_list, &hcptr->node, hcptr);
  
  return hcptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int httpc_vconnect(struct httpc *hcptr, void *cb, va_list args)
{
  httpc_vset_args(hcptr, args);
  
  hcptr->callback = cb;
  hcptr->status = HTTPC_CONNECTING;
  
  return connect_start(hcptr->connect);
}

int httpc_connect(struct httpc *hcptr, void *cb, ...)
{
  va_list args;
  int     ret;
  
  va_start(args, cb);
  ret = httpc_vconnect(hcptr, cb, args);
  va_end(args);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int httpc_url_parse(struct httpc *hcptr)
{
  size_t i;

  i = httpc_parse_protocol(hcptr);
  i = httpc_parse_host(hcptr, i);
  i = httpc_parse_port(hcptr, i);
  i = httpc_parse_location(hcptr, i);
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
size_t httpc_url_encode(char *to, const char *from, size_t n, int loc)
{
  size_t si;
  size_t di;
  
  for(si = 0, di = 0; from[si] && di + 4 < n; si++)
  {
    if(isalnum(from[si]) || (loc && from[si] == '/') || 
       (loc && from[si] == '.') ||
       from[si] == '-' || from[si] == '_')
    {
      to[di++] = from[si];
    }
    else if(from[si] == ' ' && !loc)
    {
      to[di++] = '+';
    }
    else
    {
      to[di++] = '%';
      to[di++] = httpc_hexchars[(uint32_t)(from[si] >> 4)];
      to[di++] = httpc_hexchars[(uint32_t)(from[si] & 0x0f)];
    }
  }
  
  to[di] = '\0';
  
  return di;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int httpc_url_build(struct httpc *hcptr)
{
  size_t i;
  
  if((hcptr->ssl == 0 && hcptr->port == 80) ||
     (hcptr->ssl == 1 && hcptr->port == 443))
  {
    i = snprintf(hcptr->url, sizeof(hcptr->url), "%s://%s",
                 hcptr->protocol, hcptr->host);
  }
  else
  {
    i = snprintf(hcptr->url, sizeof(hcptr->url), "%s://%s:%u",
                 hcptr->protocol, hcptr->host, (unsigned int)hcptr->port);
  }
  
  hcptr->loc = &hcptr->url[i];
  i += httpc_url_encode(&hcptr->url[i], hcptr->location, sizeof(hcptr->url) - i - 1, 1);  
  
  if(hcptr->type == HTTPC_TYPE_GET)
  {
    httpc_var_build(hcptr);
    
    if(hcptr->content_length)
    {
      hcptr->url[i++] = '?';
      i += strlcpy(&hcptr->url[i], hcptr->content, sizeof(hcptr->url) - i - 1);
    }
  }
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int httpc_update(struct httpc *hcptr)
{
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_clear(struct httpc *hcptr)
{
  struct httpc_var *hvptr;
  struct node      *next;
  
  if(hcptr->content)
  {
    mem_dynamic_free(&httpc_dheap, hcptr->content);
    hcptr->content = NULL;
    hcptr->content_length = 0;
  }
  
  if(hcptr->data)
  {
    mem_dynamic_free(&httpc_dheap, hcptr->data);
    hcptr->data = NULL;
    hcptr->data_length = 0;
  }
  
  dlink_foreach_safe(&hcptr->vars, hvptr, next)
  {
    dlink_delete(&hcptr->vars, &hvptr->node);
    mem_static_free(&httpc_vars_heap, hvptr);
  }
  
  dlink_foreach_safe(&hcptr->header, hvptr, next)
  {
    dlink_delete(&hcptr->header, &hvptr->node);
    mem_static_free(&httpc_vars_heap, hvptr);
  }  
  
  if(hcptr->connect)
    connect_cancel(hcptr->connect);
  
  hcptr->status = 0;
  hcptr->chunked = 0;
  hcptr->loc = NULL;
}

/* -------------------------------------------------------------------------- *
 * Remove a httpc.                                                            *
 * -------------------------------------------------------------------------- */
void httpc_delete(struct httpc *hcptr)
{
  httpc_clear(hcptr);
  
  if(hcptr->connect)
  {
    connect_delete(hcptr->connect);
    hcptr->connect = NULL;
  }
  
  dlink_delete(&httpc_list, &hcptr->node);
  
  mem_static_free(&httpc_heap, hcptr);
}

/* -------------------------------------------------------------------------- *
 * Send HTTP request                                                          *
 * -------------------------------------------------------------------------- */
void httpc_send(struct httpc *hcptr)
{
  httpc_url_build(hcptr);
  
  io_puts(hcptr->fd, "%s %s HTTP/1.1\r",
          (hcptr->type == HTTPC_TYPE_GET ? "GET" : "POST"),
          hcptr->loc ? hcptr->loc : "/");
  io_puts(hcptr->fd, "Host: %s\r", hcptr->host);
  io_puts(hcptr->fd, "User-Agent: %s\r", httpc_version);
  io_puts(hcptr->fd, "Connection: close\r");
  io_puts(hcptr->fd, "Cache-Control: no-cache\r");
  
  if(hcptr->type == HTTPC_TYPE_POST && hcptr->content)
  {
    io_puts(hcptr->fd, "Content-Type: multipart/form-data\r");
    io_puts(hcptr->fd, "Content-Length: %u\r\n\r", hcptr->content_length);
    io_write(hcptr->fd, hcptr->content, hcptr->content_length);
  }
  else
  {
    io_puts(hcptr->fd, "\r");
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_recv_header(struct httpc *hcptr, char *s)
{
  struct httpc_var *hvptr;
  size_t            n;
  char             *hdv[3];
  char             *p;
  
  n = strtokenize(s, hdv, 2);
  
/*  debug(httpc_log, "receiving header data: %s", s);*/
  
  if(n == 2)
  {
    if(!strncmp(hdv[0], "HTTP", 4))
    {
      hcptr->code = (int)strtoul(hdv[1], &p, 10);
      strlcpy(hcptr->error, &p[1], sizeof(hcptr->error));
      
      return;
    }
    else if((p = strchr(hdv[0], ':')))
    {
      *p = '\0';
      
      httpc_header_set(hcptr, hdv[0], hdv[1]);
      
/*      debug(httpc_log, "%s: %s", hdv[0], hdv[1]);*/
      
      return;
    }
  }
    
  hcptr->status = HTTPC_DATA;
  hcptr->chunked = 0;
  hcptr->data_length = 0;
  hcptr->chunk_length = 0;
  
  
  hvptr = httpc_header_find(hcptr, "Transfer-Encoding");
  
  if(hvptr && !strcmp(hvptr->value, "chunked"))
  {
/*    debug(httpc_log, "chunked transfer mode");*/
    hcptr->chunked = 1;
  }
  else
  {      
/*    debug(httpc_log, "linear transfer mode");*/
    if((hvptr = httpc_header_find(hcptr, "Content-length")))
      hcptr->data_length = strtoul(hvptr->value, NULL, 10);
  }    
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_recv_body(struct httpc *hcptr)
{
  if(hcptr->chunked)
  {
    if(hcptr->chunk_length == 0 && io_list[hcptr->fd].recvq.lines)
    {
      char lala[32];
      
      lala[0] = '\0';
      
      io_gets(hcptr->fd, lala, sizeof(lala));
      
      hcptr->chunk_length = strtoul(lala, NULL, 16);
      
/*      debug(httpc_log, "junk length: %u", hcptr->chunk_length);*/
      
      if(hcptr->chunk_length == 0)
        hcptr->status = HTTPC_DONE;
    }
    else if(hcptr->chunk_length)
    {
      if(hcptr->chunk_length <= io_list[hcptr->fd].recvq.size)
      {
        char junk[32];
        
        hcptr->data = mem_dynamic_realloc(&httpc_dheap, hcptr->data,
                                          hcptr->data_length + hcptr->chunk_length);
        io_read(hcptr->fd, &hcptr->data[hcptr->data_length], hcptr->chunk_length);
        hcptr->data_length += hcptr->chunk_length;
        hcptr->chunk_length = 0;
  
        io_gets(hcptr->fd, junk, 32);
      }
    }
  }
  else
  {
    if(hcptr->data_length <= io_list[hcptr->fd].recvq.size)
    {
      hcptr->data = mem_dynamic_alloc(&httpc_dheap, hcptr->data_length);
      io_read(hcptr->fd, hcptr->data, hcptr->data_length);
      
      hcptr->status = HTTPC_DONE;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Receive HTTP data                                                          *
 * -------------------------------------------------------------------------- */
void httpc_recv(int fd, struct httpc *hcptr)
{
  char buf[1024];
  int  n;
  
  if(hcptr->status == HTTPC_SENT)
    hcptr->status = HTTPC_HEADER;
 
  if(hcptr->status == HTTPC_HEADER)
  {
    while(io_list[fd].recvq.lines && hcptr->status == HTTPC_HEADER)
    {
      n = io_gets(fd, buf, sizeof(buf));
      
      httpc_recv_header(hcptr, buf);
    }
    
    return;
  }
  else if(hcptr->status == HTTPC_DATA)
  {
    if(io_list[fd].status.err)
    {
/*      debug(httpc_log, "receiving data");*/
     
      while(io_list[fd].recvq.size && hcptr->status != HTTPC_DONE)
        httpc_recv_body(hcptr);
    }
  }  
  
  if(hcptr->status == HTTPC_DONE)
  {
    hcptr->callback(hcptr,
                    hcptr->args[0], hcptr->args[1],
                    hcptr->args[1], hcptr->args[2]);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc *httpc_find_name(const char *name)
{
  struct httpc *hcptr;
  uint32_t       hash;
    
  hash = strhash(name);
  
  dlink_foreach(&httpc_list, hcptr)
  {
    if(hcptr->nhash == hash)
    {
      if(!strcmp(hcptr->name, name))
        return hcptr;
    }
  }
  
  return NULL;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc *httpc_find_id(uint32_t id)
{
  struct httpc *hcptr;
  
  dlink_foreach(&httpc_list, hcptr)
  {
    if(hcptr->id == id)
      return hcptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_vset_args(struct httpc *httpc, va_list args)
{
  httpc->args[0] = va_arg(args, void *);
  httpc->args[1] = va_arg(args, void *);
  httpc->args[2] = va_arg(args, void *);
  httpc->args[3] = va_arg(args, void *);
}

void httpc_set_args(struct httpc *httpc, ...)
{
  va_list args;
  
  va_start(args, httpc);
  httpc_vset_args(httpc, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void httpc_set_name(struct httpc *hcptr, const char *name)
{
  strlcpy(hcptr->name, name, sizeof(hcptr->name));
  hcptr->nhash = strihash(hcptr->name);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc *httpc_pop(struct httpc *httpc)
{
  if(httpc)
  {
    if(!httpc->refcount)
      log(httpc_log, L_warning, "Poping deprecated httpc: %s",
          httpc->name);

    httpc->refcount++;
  }

  return httpc;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc *httpc_push(struct httpc **httpcptr)
{
  if(*httpcptr)
  {
    if((*httpcptr)->refcount == 0)
    {
      log(httpc_log, L_warning, "Trying to push deprecated user: %s",
          (*httpcptr)->name);
    }
    else
    {
      if(--(*httpcptr)->refcount == 0)
        httpc_delete(*httpcptr);
      
      (*httpcptr) = NULL;
    }
  }
  
  return *httpcptr;
}

/* -------------------------------------------------------------------------- *
 * Dump httpc list and heap.                                                  *
 * -------------------------------------------------------------------------- */
void httpc_dump(struct httpc *hcptr)
{
  if(hcptr == NULL)
  {
    dump(httpc_log, "[================ httpc summary ================]");
    
    dlink_foreach(&httpc_list, hcptr)
    {
      dump(httpc_log, " #%u: [%u] %-20s (%i)",
           hcptr->id, hcptr->refcount, hcptr->name, hcptr->fd);
    }

    dump(httpc_log, "[============= end of httpc summary ============]");
  }
  else
  {
    char *p;
    char *next;
    
    dump(httpc_log, "[================= httpc dump =================]");
    
    dump(httpc_log, "         id: #%u", hcptr->id);
    dump(httpc_log, "   refcount: %u", hcptr->refcount);
    dump(httpc_log, "      nhash: %p", hcptr->nhash);
    dump(httpc_log, "         fd: %i", hcptr->fd);
    dump(httpc_log, "       name: %s", hcptr->name);
    
    if(hcptr->data && hcptr->data_length)
    {
      dump(httpc_log, "------------------ httpc data ------------------");
      
      p = hcptr->data;
      
      do
      {
        next = strchr(p, '\n');
      
        if(next) *next = '\0';
        
        dump(httpc_log, "%s", p);
        
        if(next) {
          *next = '\n';
          next++;
        }
        
      } while((p = next));
    }
    
    dump(httpc_log, "[============== end of httpc dump =============]");
  }
}
