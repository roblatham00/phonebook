/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _ADMIN_H
#define _ADMIN_H

#include "types.h"
#include "YP/YP-admin.h"

typedef struct YP_admin {
   margo_instance_id mid;
   hg_id_t           create_phonebook_id;
   hg_id_t           open_phonebook_id;
   hg_id_t           close_phonebook_id;
   hg_id_t           destroy_phonebook_id;
   hg_id_t           list_phonebooks_id;
} YP_admin;

#endif
