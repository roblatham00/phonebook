/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "YP/YP-client.h"
#include "YP/YP-phonebook.h"

typedef struct YP_client {
   margo_instance_id mid;
   hg_id_t           hello_id;
   hg_id_t           sum_id;
   uint64_t          num_phonebook_handles;
} YP_client;

typedef struct YP_phonebook_handle {
    YP_client_t      client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
    YP_phonebook_id_t phonebook_id;
} YP_phonebook_handle;

#endif
