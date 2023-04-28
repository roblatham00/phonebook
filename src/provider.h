/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __PROVIDER_H
#define __PROVIDER_H

#include <margo.h>
#include <uuid.h>
#include <json-c/json.h>
#include "YP/YP-backend.h"
#include "uthash.h"

typedef struct YP_phonebook {
    YP_backend_impl* fn;  // pointer to function mapping for this backend
    void*               ctx; // context required by the backend
    YP_phonebook_id_t id;  // identifier of the backend
    UT_hash_handle      hh;  // handle for uthash
} YP_phonebook;

typedef struct YP_provider {
    /* Margo/Argobots/Mercury environment */
    margo_instance_id   mid;                 // Margo instance
    uint16_t            provider_id;         // Provider id
    ABT_pool            pool;                // Pool on which to post RPC requests
    char*               token;               // Security token
    /* Resources and backend types */
    size_t               num_backend_types; // number of backend types
    YP_backend_impl** backend_types;     // array of pointers to backend types
    size_t               num_phonebooks;     // number of phonebooks
    YP_phonebook*      phonebooks;         // hash of phonebooks by uuid
    /* RPC identifiers for admins */
    hg_id_t create_phonebook_id;
    hg_id_t open_phonebook_id;
    hg_id_t close_phonebook_id;
    hg_id_t destroy_phonebook_id;
    hg_id_t list_phonebooks_id;
    /* RPC identifiers for clients */
    hg_id_t hello_id;
    hg_id_t sum_id;
    /* ... add other RPC identifiers here ... */
} YP_provider;

#endif
