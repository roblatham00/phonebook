/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __YP_COMMON_H
#define __YP_COMMON_H

#include <uuid.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by YP functions.
 */
typedef enum YP_return_t {
    YP_SUCCESS,
    YP_ERR_ALLOCATION,        /* Allocation error */
    YP_ERR_INVALID_ARGS,      /* Invalid argument */
    YP_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    YP_ERR_INVALID_PHONEBOOK,  /* Invalid phonebook id */
    YP_ERR_INVALID_BACKEND,   /* Invalid backend type */
    YP_ERR_INVALID_CONFIG,    /* Invalid configuration */
    YP_ERR_INVALID_TOKEN,     /* Invalid token */
    YP_ERR_FROM_MERCURY,      /* Mercurt error */
    YP_ERR_FROM_ARGOBOTS,     /* Argobots error */
    YP_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    YP_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    YP_ERR_OTHER              /* Other error */
} YP_return_t;

/**
 * @brief Identifier for a phonebook.
 */
typedef struct YP_phonebook_id_t {
    uuid_t uuid;
} YP_phonebook_id_t;

/**
 * @brief Converts a YP_phonebook_id_t into a string.
 *
 * @param id Id to convert
 * @param out[37] Resulting null-terminated string
 */
static inline void YP_phonebook_id_to_string(
        YP_phonebook_id_t id,
        char out[37]) {
    uuid_unparse(id.uuid, out);
}

/**
 * @brief Converts a string into a YP_phonebook_id_t. The string
 * should be a 36-characters string + null terminator.
 *
 * @param in input string
 * @param id resulting id
 */
static inline void YP_phonebook_id_from_string(
        const char* in,
        YP_phonebook_id_t* id) {
    uuid_parse(in, id->uuid);
}

#ifdef __cplusplus
}
#endif

#endif
