/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "YP/YP-admin.h"

YP_return_t YP_admin_init(margo_instance_id mid, YP_admin_t* admin)
{
    YP_admin_t a = (YP_admin_t)calloc(1, sizeof(*a));
    if(!a) return YP_ERR_ALLOCATION;

    a->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "YP_create_phonebook", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "YP_create_phonebook", &a->create_phonebook_id, &flag);
        margo_registered_name(mid, "YP_open_phonebook", &a->open_phonebook_id, &flag);
        margo_registered_name(mid, "YP_close_phonebook", &a->close_phonebook_id, &flag);
        margo_registered_name(mid, "YP_destroy_phonebook", &a->destroy_phonebook_id, &flag);
        margo_registered_name(mid, "YP_list_phonebooks", &a->list_phonebooks_id, &flag);
        /* Get more existing RPCs... */
    } else {
        a->create_phonebook_id =
            MARGO_REGISTER(mid, "YP_create_phonebook",
            create_phonebook_in_t, create_phonebook_out_t, NULL);
        a->open_phonebook_id =
            MARGO_REGISTER(mid, "YP_open_phonebook",
            open_phonebook_in_t, open_phonebook_out_t, NULL);
        a->close_phonebook_id =
            MARGO_REGISTER(mid, "YP_close_phonebook",
            close_phonebook_in_t, close_phonebook_out_t, NULL);
        a->destroy_phonebook_id =
            MARGO_REGISTER(mid, "YP_destroy_phonebook",
            destroy_phonebook_in_t, destroy_phonebook_out_t, NULL);
        a->list_phonebooks_id =
            MARGO_REGISTER(mid, "YP_list_phonebooks",
            list_phonebooks_in_t, list_phonebooks_out_t, NULL);
        /* Register more RPCs ... */
    }

    *admin = a;
    return YP_SUCCESS;
}

YP_return_t YP_admin_finalize(YP_admin_t admin)
{
    free(admin);
    return YP_SUCCESS;
}

YP_return_t YP_create_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        YP_phonebook_id_t* id)
{
    hg_handle_t h;
    create_phonebook_in_t  in;
    create_phonebook_out_t out;
    hg_return_t hret;
    YP_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->create_phonebook_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != YP_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return YP_SUCCESS;
}

YP_return_t YP_open_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        YP_phonebook_id_t* id)
{
    hg_handle_t h;
    open_phonebook_in_t  in;
    open_phonebook_out_t out;
    hg_return_t hret;
    YP_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->open_phonebook_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != YP_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return YP_SUCCESS;
}

YP_return_t YP_close_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t id)
{
    hg_handle_t h;
    close_phonebook_in_t  in;
    close_phonebook_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->close_phonebook_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

YP_return_t YP_destroy_phonebook(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t id)
{
    hg_handle_t h;
    destroy_phonebook_in_t  in;
    destroy_phonebook_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->destroy_phonebook_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

YP_return_t YP_list_phonebooks(
        YP_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        YP_phonebook_id_t* ids,
        size_t* count)
{
    hg_handle_t h;
    list_phonebooks_in_t  in;
    list_phonebooks_out_t out;
    YP_return_t ret;
    hg_return_t hret;

    in.token  = (char*)token;
    in.max_ids = *count;

    hret = margo_create(admin->mid, address, admin->list_phonebooks_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == YP_SUCCESS) {
        *count = out.count;
        memcpy(ids, out.ids, out.count*sizeof(*ids));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
