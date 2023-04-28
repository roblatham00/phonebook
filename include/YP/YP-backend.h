/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_BACKEND_H
#define __YP_BACKEND_H

#include <YP/YP-server.h>
#include <YP/YP-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef YP_return_t (*YP_backend_create_fn)(YP_provider_t, const char*, void**);
typedef YP_return_t (*YP_backend_open_fn)(YP_provider_t, const char*, void**);
typedef YP_return_t (*YP_backend_close_fn)(void*);
typedef YP_return_t (*YP_backend_destroy_fn)(void*);
typedef char* (*YP_backend_get_config_fn)(void*);

/**
 * @brief Implementation of an YP backend.
 */
typedef struct YP_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    YP_backend_create_fn     create_phonebook;
    YP_backend_open_fn       open_phonebook;
    YP_backend_close_fn      close_phonebook;
    YP_backend_destroy_fn    destroy_phonebook;
    YP_backend_get_config_fn get_config;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
} YP_backend_impl;

/**
 * @brief Registers a backend implementation to be used by the
 * specified provider.
 *
 * Note: the backend implementation will not be copied; it is
 * therefore important that it stays valid in memory until the
 * provider is destroyed.
 *
 * @param provider provider.
 * @param backend_impl backend implementation.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_provider_register_backend(
        YP_provider_t provider,
        YP_backend_impl* backend_impl);

#ifdef __cplusplus
}
#endif

#endif
