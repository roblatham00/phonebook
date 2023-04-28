/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "YP/YP-client.h"

YP_return_t YP_client_init(margo_instance_id mid, YP_client_t* client)
{
    YP_client_t c = (YP_client_t)calloc(1, sizeof(*c));
    if(!c) return YP_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "YP_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "YP_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "YP_hello", &c->hello_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "YP_sum", sum_in_t, sum_out_t, NULL);
        c->hello_id = MARGO_REGISTER(mid, "YP_hello", hello_in_t, void, NULL);
        margo_registered_disable_response(mid, c->hello_id, HG_TRUE);
    }

    *client = c;
    return YP_SUCCESS;
}

YP_return_t YP_client_finalize(YP_client_t client)
{
    if(client->num_phonebook_handles != 0) {
        margo_warning(client->mid,
            "%ld phonebook handles not released when YP_client_finalize was called",
            client->num_phonebook_handles);
    }
    free(client);
    return YP_SUCCESS;
}

YP_return_t YP_phonebook_handle_create(
        YP_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        YP_phonebook_id_t phonebook_id,
        YP_phonebook_handle_t* handle)
{
    if(client == YP_CLIENT_NULL)
        return YP_ERR_INVALID_ARGS;

    YP_phonebook_handle_t rh =
        (YP_phonebook_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return YP_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return YP_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->phonebook_id = phonebook_id;
    rh->refcount    = 1;

    client->num_phonebook_handles += 1;

    *handle = rh;
    return YP_SUCCESS;
}

YP_return_t YP_phonebook_handle_ref_incr(
        YP_phonebook_handle_t handle)
{
    if(handle == YP_PHONEBOOK_HANDLE_NULL)
        return YP_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return YP_SUCCESS;
}

YP_return_t YP_phonebook_handle_release(YP_phonebook_handle_t handle)
{
    if(handle == YP_PHONEBOOK_HANDLE_NULL)
        return YP_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_phonebook_handles -= 1;
        free(handle);
    }
    return YP_SUCCESS;
}

YP_return_t YP_say_hello(YP_phonebook_handle_t handle)
{
    hg_handle_t   h;
    hello_in_t     in;
    hg_return_t ret;

    memcpy(&in.phonebook_id, &(handle->phonebook_id), sizeof(in.phonebook_id));

    ret = margo_create(handle->client->mid, handle->addr, handle->client->hello_id, &h);
    if(ret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return YP_ERR_FROM_MERCURY;
    }

    margo_destroy(h);
    return YP_SUCCESS;
}

YP_return_t YP_compute_sum(
        YP_phonebook_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    YP_return_t ret;

    memcpy(&in.phonebook_id, &(handle->phonebook_id), sizeof(in.phonebook_id));
    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return YP_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == YP_SUCCESS)
        *result = out.result;

finish:
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
