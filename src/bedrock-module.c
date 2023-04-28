/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "YP/YP-server.h"
#include "YP/YP-client.h"
#include "YP/YP-admin.h"
#include "YP/YP-provider-handle.h"
#include "client.h"
#include <string.h>

static int YP_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct YP_provider_args YP_args = { 0 };
    YP_args.config = bedrock_args_get_config(args);
    YP_args.pool   = bedrock_args_get_pool(args);

    return YP_provider_register(mid, provider_id, &YP_args,
                                   (YP_provider_t*)provider);
}

static int YP_deregister_provider(
        bedrock_module_provider_t provider)
{
    return YP_provider_destroy((YP_provider_t)provider);
}

static char* YP_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    return YP_provider_get_config(provider);
}

static int YP_init_client(
        bedrock_args_t args,
        bedrock_module_client_t* client)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    return YP_client_init(mid, (YP_client_t*)client);
}

static int YP_finalize_client(
        bedrock_module_client_t client)
{
    return YP_client_finalize((YP_client_t)client);
}

static char* YP_get_client_config(
        bedrock_module_client_t client) {
    (void)client;
    // TODO
    return strdup("{}");
}

static int YP_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    YP_client_t c = (YP_client_t)client;
    YP_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int YP_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    YP_provider_handle_t tmp = (YP_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module YP = {
    .register_provider       = YP_register_provider,
    .deregister_provider     = YP_deregister_provider,
    .get_provider_config     = YP_get_provider_config,
    .init_client             = YP_init_client,
    .finalize_client         = YP_finalize_client,
    .get_client_config       = YP_get_client_config,
    .create_provider_handle  = YP_create_provider_handle,
    .destroy_provider_handle = YP_destroy_provider_handle,
    .provider_dependencies   = NULL,
    .client_dependencies     = NULL
};

BEDROCK_REGISTER_MODULE(YP, YP)
