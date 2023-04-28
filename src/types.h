/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _PARAMS_H
#define _PARAMS_H

#include <stdlib.h>
#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc.h>
#include <mercury_proc_string.h>
#include "YP/YP-common.h"

static inline hg_return_t hg_proc_YP_phonebook_id_t(hg_proc_t proc, YP_phonebook_id_t *id);

/* Admin RPC types */

MERCURY_GEN_PROC(create_phonebook_in_t,
        ((hg_string_t)(type))\
        ((hg_string_t)(config))\
        ((hg_string_t)(token)))

MERCURY_GEN_PROC(create_phonebook_out_t,
        ((int32_t)(ret))\
        ((YP_phonebook_id_t)(id)))

MERCURY_GEN_PROC(open_phonebook_in_t,
        ((hg_string_t)(type))\
        ((hg_string_t)(config))\
        ((hg_string_t)(token)))

MERCURY_GEN_PROC(open_phonebook_out_t,
        ((int32_t)(ret))\
        ((YP_phonebook_id_t)(id)))

MERCURY_GEN_PROC(close_phonebook_in_t,
        ((hg_string_t)(token))\
        ((YP_phonebook_id_t)(id)))

MERCURY_GEN_PROC(close_phonebook_out_t,
        ((int32_t)(ret)))

MERCURY_GEN_PROC(destroy_phonebook_in_t,
        ((hg_string_t)(token))\
        ((YP_phonebook_id_t)(id)))

MERCURY_GEN_PROC(destroy_phonebook_out_t,
        ((int32_t)(ret)))

MERCURY_GEN_PROC(list_phonebooks_in_t,
        ((hg_string_t)(token))\
        ((hg_size_t)(max_ids)))

typedef struct list_phonebooks_out_t {
    int32_t ret;
    hg_size_t count;
    YP_phonebook_id_t* ids;
} list_phonebooks_out_t;

static inline hg_return_t hg_proc_list_phonebooks_out_t(hg_proc_t proc, void *data)
{
    list_phonebooks_out_t* out = (list_phonebooks_out_t*)data;
    hg_return_t ret;

    ret = hg_proc_hg_int32_t(proc, &(out->ret));
    if(ret != HG_SUCCESS) return ret;

    ret = hg_proc_hg_size_t(proc, &(out->count));
    if(ret != HG_SUCCESS) return ret;

    switch(hg_proc_get_op(proc)) {
    case HG_DECODE:
        out->ids = (YP_phonebook_id_t*)calloc(out->count, sizeof(*(out->ids)));
        /* fall through */
    case HG_ENCODE:
        if(out->ids)
            ret = hg_proc_memcpy(proc, out->ids, sizeof(*(out->ids))*out->count);
        break;
    case HG_FREE:
        free(out->ids);
        break;
    }
    return ret;
}

/* Client RPC types */

MERCURY_GEN_PROC(hello_in_t,
        ((YP_phonebook_id_t)(phonebook_id)))

MERCURY_GEN_PROC(sum_in_t,
        ((YP_phonebook_id_t)(phonebook_id))\
        ((int32_t)(x))\
        ((int32_t)(y)))

MERCURY_GEN_PROC(sum_out_t,
        ((int32_t)(result))\
        ((int32_t)(ret)))

/* Extra hand-coded serialization functions */

static inline hg_return_t hg_proc_YP_phonebook_id_t(
        hg_proc_t proc, YP_phonebook_id_t *id)
{
    return hg_proc_memcpy(proc, id, sizeof(*id));
}

#endif
