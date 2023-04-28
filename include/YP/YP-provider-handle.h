/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_PROVIDER_HANDLE_H
#define __YP_PROVIDER_HANDLE_H

#include <margo.h>
#include <YP/YP-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct YP_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct YP_provider_handle* YP_provider_handle_t;
#define YP_PROVIDER_HANDLE_NULL ((YP_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
