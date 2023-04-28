/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <YP/YP-client.h>
#include <YP/YP-phonebook.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr,"Usage: %s <server address> <provider id> <phonebook id>\n", argv[0]);
        exit(-1);
    }

    YP_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    const char* id_str       = argv[3];
    if(strlen(id_str) != 36) {
        FATAL(MARGO_INSTANCE_NULL,"id should be 36 character long");
    }

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    margo_set_log_level(mid, MARGO_LOG_INFO);

    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed for address %s", svr_addr_str);
    }

    YP_client_t YP_clt;
    YP_phonebook_handle_t YP_rh;

    margo_info(mid, "Creating YP client");
    ret = YP_client_init(mid, &YP_clt);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_client_init failed (ret = %d)", ret);
    }

    YP_phonebook_id_t phonebook_id;
    YP_phonebook_id_from_string(id_str, &phonebook_id);

    margo_info(mid, "Creating phonebook handle for phonebook %s", id_str);
    ret = YP_phonebook_handle_create(
            YP_clt, svr_addr, provider_id,
            phonebook_id, &YP_rh);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_phonebook_handle_create failed (ret = %d)", ret);
    }

    margo_info(mid, "Saying Hello to server");
    ret = YP_say_hello(YP_rh);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_say_hello failed (ret = %d)", ret);
    }

    margo_info(mid, "Computing sum");
    int32_t result;
    ret = YP_compute_sum(YP_rh, 45, 23, &result);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_compute_sum failed (ret = %d)", ret);
    }
    margo_info(mid, "45 + 23 = %d", result);

    margo_info(mid, "Releasing phonebook handle");
    ret = YP_phonebook_handle_release(YP_rh);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_phonebook_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = YP_client_finalize(YP_clt);
    if(ret != YP_SUCCESS) {
        FATAL(mid,"YP_client_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
