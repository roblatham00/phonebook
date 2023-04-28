/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <json-c/json.h>
#include "YP/YP-backend.h"
#include "../provider.h"
#include "dummy-backend.h"

typedef struct dummy_context {
    struct json_object* config;
    /* ... */
} dummy_context;

static YP_return_t dummy_create_phonebook(
        YP_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;
    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return YP_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    *context = (void*)ctx;
    return YP_SUCCESS;
}

static YP_return_t dummy_open_phonebook(
        YP_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;

    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return YP_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    *context = (void*)ctx;
    return YP_SUCCESS;
}

static YP_return_t dummy_close_phonebook(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    free(context);
    return YP_SUCCESS;
}

static YP_return_t dummy_destroy_phonebook(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    free(context);
    return YP_SUCCESS;
}

static char* dummy_get_config(void* ctx)
{
    (void)ctx;
    return strdup("{}");
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy phonebook\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static YP_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_phonebook  = dummy_create_phonebook,
    .open_phonebook    = dummy_open_phonebook,
    .close_phonebook   = dummy_close_phonebook,
    .destroy_phonebook = dummy_destroy_phonebook,
    .get_config       = dummy_get_config,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum
};

YP_return_t YP_provider_register_dummy_backend(YP_provider_t provider)
{
    return YP_provider_register_backend(provider, &dummy_backend);
}
