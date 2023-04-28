/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_PHONEBOOK_H
#define __YP_PHONEBOOK_H

#include <margo.h>
#include <YP/YP-common.h>
#include <YP/YP-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct YP_phonebook_handle *YP_phonebook_handle_t;
#define YP_PHONEBOOK_HANDLE_NULL ((YP_phonebook_handle_t)NULL)

/**
 * @brief Creates a YP phonebook handle.
 *
 * @param[in] client YP client responsible for the phonebook handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle phonebook handle
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_phonebook_handle_create(
        YP_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        YP_phonebook_id_t phonebook_id,
        YP_phonebook_handle_t* handle);

/**
 * @brief Increments the reference counter of a phonebook handle.
 *
 * @param handle phonebook handle
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_phonebook_handle_ref_incr(
        YP_phonebook_handle_t handle);

/**
 * @brief Releases the phonebook handle. This will decrement the
 * reference counter, and free the phonebook handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle phonebook handle to release.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_phonebook_handle_release(YP_phonebook_handle_t handle);

/**
 * @brief Makes the target YP phonebook print Hello World.
 *
 * @param[in] handle phonebook handle.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_say_hello(YP_phonebook_handle_t handle);

/**
 * @brief Makes the target YP phonebook compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle phonebook handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_compute_sum(
        YP_phonebook_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

#ifdef __cplusplus
}
#endif

#endif
