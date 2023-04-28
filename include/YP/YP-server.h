/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_SERVER_H
#define __YP_SERVER_H

#include <YP/YP-common.h>
#include <margo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YP_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct YP_provider* YP_provider_t;
#define YP_PROVIDER_NULL ((YP_provider_t)NULL)
#define YP_PROVIDER_IGNORE ((YP_provider_t*)NULL)

struct YP_provider_args {
    const char*        token;  // Security token
    const char*        config; // JSON configuration
    ABT_pool           pool;   // Pool used to run RPCs
    // ...
};

#define YP_PROVIDER_ARGS_INIT { \
    /* .token = */ NULL, \
    /* .config = */ NULL, \
    /* .pool = */ ABT_POOL_NULL \
}

/**
 * @brief Creates a new YP provider. If YP_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct YP_provider_args* args,
        YP_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_provider_destroy(
        YP_provider_t provider);

/**
 * @brief Returns a JSON-formatted configuration of the provider.
 *
 * The caller is responsible for freeing the returned pointer.
 *
 * @param provider Alpha provider
 *
 * @return a heap-allocated JSON string or NULL in case of an error.
 */
char* YP_provider_get_config(
        YP_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif
