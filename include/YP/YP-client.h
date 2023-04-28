/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_CLIENT_H
#define __YP_CLIENT_H

#include <margo.h>
#include <YP/YP-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct YP_client* YP_client_t;
#define YP_CLIENT_NULL ((YP_client_t)NULL)

/**
 * @brief Creates a YP client.
 *
 * @param[in] mid Margo instance
 * @param[out] client YP client
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_client_init(margo_instance_id mid, YP_client_t* client);

/**
 * @brief Finalizes a YP client.
 *
 * @param[in] client YP client to finalize
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_client_finalize(YP_client_t client);

#ifdef __cplusplus
}
#endif

#endif
