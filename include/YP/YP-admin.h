/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_ADMIN_H
#define __YP_ADMIN_H

#include <margo.h>
#include <YP/YP-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct YP_admin* YP_admin_t;
#define YP_ADMIN_NULL ((YP_admin_t)NULL)

#define YP_PHONEBOOK_ID_IGNORE ((YP_phonebook_id_t*)NULL)

/**
 * @brief Creates a YP admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin YP admin
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_admin_init(margo_instance_id mid, YP_admin_t* admin);

/**
 * @brief Finalizes a YP admin.
 *
 * @param[in] admin YP admin to finalize
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_admin_finalize(YP_admin_t admin);

/**
 * @brief Requests the provider to create a phonebook of the
 * specified type and configuration and return a phonebook id.
 *
 * @param[in] admin YP admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] type type of phonebook to create.
 * @param[in] config Configuration.
 * @param[out] id resulting phonebook id.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_create_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        YP_phonebook_id_t* id);

/**
 * @brief Requests the provider to open an existing phonebook of the
 * specified type and configuration and return a phonebook id.
 *
 * @param[in] admin YP admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] type type of phonebook to open.
 * @param[in] config Configuration.
 * @param[out] id resulting phonebook id.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_open_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        YP_phonebook_id_t* id);

/**
 * @brief Requests the provider to close a phonebook it is managing.
 *
 * @param[in] admin YP admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting phonebook id.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_close_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t id);

/**
 * @brief Requests the provider to destroy a phonebook it is managing.
 *
 * @param[in] admin YP admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting phonebook id.
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_destroy_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t id);

/**
 * @brief Lists the ids of phonebooks available on the provider.
 *
 * @param[in] admin YP admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[out] ids array of phonebook ids.
 * @param[inout] count size of the array (in), number of ids returned (out).
 *
 * @return YP_SUCCESS or error code defined in YP-common.h
 */
YP_return_t YP_list_phonebooks(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t* ids,
        size_t* count);

#if defined(__cplusplus)
}
#endif

#endif
