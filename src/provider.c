/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "YP/YP-server.h"
#include "provider.h"
#include "types.h"

// backends that we want to add at compile time
#include "dummy/dummy-backend.h"

static void YP_finalize_provider(void* p);

/* Functions to manipulate the hash of phonebooks */
static inline YP_phonebook* find_phonebook(
        YP_provider_t provider,
        const YP_phonebook_id_t* id);

static inline YP_return_t add_phonebook(
        YP_provider_t provider,
        YP_phonebook* phonebook);

static inline YP_return_t remove_phonebook(
        YP_provider_t provider,
        const YP_phonebook_id_t* id,
        int close_phonebook);

static inline void remove_all_phonebooks(
        YP_provider_t provider);

/* Functions to manipulate the list of backend types */
static inline YP_backend_impl* find_backend_impl(
        YP_provider_t provider,
        const char* name);

static inline YP_return_t add_backend_impl(
        YP_provider_t provider,
        YP_backend_impl* backend);

/* Function to check the validity of the token sent by an admin
 * (returns 0 is the token is incorrect) */
static inline int check_token(
        YP_provider_t provider,
        const char* token);

/* Admin RPCs */
static DECLARE_MARGO_RPC_HANDLER(YP_create_phonebook_ult)
static void YP_create_phonebook_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(YP_open_phonebook_ult)
static void YP_open_phonebook_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(YP_close_phonebook_ult)
static void YP_close_phonebook_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(YP_destroy_phonebook_ult)
static void YP_destroy_phonebook_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(YP_list_phonebooks_ult)
static void YP_list_phonebooks_ult(hg_handle_t h);

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(YP_hello_ult)
static void YP_hello_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(YP_sum_ult)
static void YP_sum_ult(hg_handle_t h);

/* add other RPC declarations here */

YP_return_t YP_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct YP_provider_args* args,
        YP_provider_t* provider)
{
    struct YP_provider_args a = YP_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    YP_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering YP provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return YP_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "YP_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return YP_ERR_INVALID_PROVIDER;
    }

    // parse json configuration
    struct json_object* config = NULL;
    if (a.config) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, a.config,
                strlen(a.config));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(mid, "JSON parse error: %s",
                    json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return YP_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
        if (!(json_object_is_type(config, json_type_object))) {
            margo_error(mid, "JSON configuration should be an object");
            json_object_put(config);
            return YP_ERR_INVALID_CONFIG;
        }
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    p = (YP_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        json_object_put(config);
        return YP_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->token = (a.token && strlen(a.token)) ? strdup(a.token) : NULL;

    /* Admin RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "YP_create_phonebook",
            create_phonebook_in_t, create_phonebook_out_t,
            YP_create_phonebook_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->create_phonebook_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "YP_open_phonebook",
            open_phonebook_in_t, open_phonebook_out_t,
            YP_open_phonebook_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->open_phonebook_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "YP_close_phonebook",
            close_phonebook_in_t, close_phonebook_out_t,
            YP_close_phonebook_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->close_phonebook_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "YP_destroy_phonebook",
            destroy_phonebook_in_t, destroy_phonebook_out_t,
            YP_destroy_phonebook_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->destroy_phonebook_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "YP_list_phonebooks",
            list_phonebooks_in_t, list_phonebooks_out_t,
            YP_list_phonebooks_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_phonebooks_id = id;

    /* Client RPCs */

    id = MARGO_REGISTER_PROVIDER(mid, "YP_hello",
            hello_in_t, void,
            YP_hello_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->hello_id = id;
    margo_registered_disable_response(mid, id, HG_TRUE);

    id = MARGO_REGISTER_PROVIDER(mid, "YP_sum",
            sum_in_t, sum_out_t,
            YP_sum_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */
    YP_provider_register_dummy_backend(p); // function from "dummy/dummy-backend.h"

    /* read the configuration to add defined phonebooks */
    struct json_object* phonebooks_array = json_object_object_get(config, "phonebooks");
    if (phonebooks_array && json_object_is_type(phonebooks_array, json_type_array)) {
        for (size_t i = 0; i < json_object_array_length(phonebooks_array); ++i) {
            struct json_object* phonebook
                = json_object_array_get_idx(phonebooks_array, i);
            if(!json_object_is_type(phonebook, json_type_object))
                continue;
            struct json_object* phonebook_type   = json_object_object_get(phonebook, "type");
            if(!json_object_is_type(phonebook_type, json_type_string)) {
                margo_error(mid, "\"type\" field in phonebook configuration should be a string");
                continue;
            }
            const char* type = json_object_get_string(phonebook_type);
            struct json_object* phonebook_config = json_object_object_get(phonebook, "config");
            YP_backend_impl* backend         = find_backend_impl(p, type);
            if(!backend) {
                margo_error(mid, "Could not find backend of type \"%s\"", type);
                continue;
            }
            /* create a uuid for the new phonebook */
            YP_phonebook_id_t id;
            uuid_generate(id.uuid);
            /* create the new phonebook's context */
            void* context = NULL;
            int ret = backend->create_phonebook(p, json_object_to_json_string(phonebook_config), &context);
            if(ret != YP_SUCCESS) {
                margo_error(mid, "Could not create phonebook, backend returned %d", ret);
                continue;
            }

            /* allocate a phonebook, set it up, and add it to the provider */
            YP_phonebook* phonebook_data = (YP_phonebook*)calloc(1, sizeof(*phonebook_data));
            phonebook_data->fn  = backend;
            phonebook_data->ctx = context;
            phonebook_data->id  = id;
            add_phonebook(p, phonebook_data);

            char id_str[37];
            YP_phonebook_id_to_string(id, id_str);
            margo_debug(mid, "Created phonebook %s of type \"%s\"", id_str, type);
        }
    }

    margo_provider_push_finalize_callback(mid, p, &YP_finalize_provider, p);

    if(provider)
        *provider = p;
    json_object_put(config);
    margo_info(mid, "YP provider registration done");
    return YP_SUCCESS;
}

static void YP_finalize_provider(void* p)
{
    YP_provider_t provider = (YP_provider_t)p;
    margo_info(provider->mid, "Finalizing YP provider");
    margo_deregister(provider->mid, provider->create_phonebook_id);
    margo_deregister(provider->mid, provider->open_phonebook_id);
    margo_deregister(provider->mid, provider->close_phonebook_id);
    margo_deregister(provider->mid, provider->destroy_phonebook_id);
    margo_deregister(provider->mid, provider->list_phonebooks_id);
    margo_deregister(provider->mid, provider->hello_id);
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    remove_all_phonebooks(provider);
    free(provider->backend_types);
    free(provider->token);
    margo_instance_id mid = provider->mid;
    free(provider);
    margo_info(mid, "YP provider successfuly finalized");
}

YP_return_t YP_provider_destroy(
        YP_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying YP provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    YP_finalize_provider(provider);
    margo_info(mid, "YP provider successfuly destroyed");
    return YP_SUCCESS;
}

char* YP_provider_get_config(YP_provider_t provider)
{
    if (!provider) return NULL;
    struct json_object* config = json_object_new_object();
    struct json_object* phonebooks_array = json_object_new_array_ext(provider->num_phonebooks);
    json_object_object_add(config, "phonebooks", phonebooks_array);

    for(size_t i = 0; i < provider->num_phonebooks; ++i) {
        struct YP_phonebook* phonebook = &provider->phonebooks[i];
        char id_str[37];
        YP_phonebook_id_to_string(phonebook->id, id_str);
        char* phonebook_config_str = (phonebook->fn->get_config)(phonebook->ctx);
        struct json_object* phonebook_config = json_object_new_object();
        json_object_object_add(phonebook_config, "__id__", json_object_new_string(id_str));
        json_object_object_add(phonebook_config, "type", json_object_new_string(phonebook->fn->name));
        json_object_object_add(phonebook_config, "config", json_tokener_parse(phonebook_config_str));
        json_object_array_add(phonebooks_array, phonebook_config);
        free(phonebook_config_str);
    }

    char* result = strdup(json_object_to_json_string(config));
    json_object_put(config);
    return result;
}

YP_return_t YP_provider_register_backend(
        YP_provider_t provider,
        YP_backend_impl* backend_impl)
{
    margo_info(provider->mid, "Adding backend implementation \"%s\" to YP provider",
             backend_impl->name);
    return add_backend_impl(provider, backend_impl);
}

static void YP_create_phonebook_ult(hg_handle_t h)
{
    hg_return_t hret;
    YP_return_t ret;
    create_phonebook_in_t  in;
    create_phonebook_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(provider->mid, "Invalid token");
        out.ret = YP_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    YP_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(provider->mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = YP_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new phonebook */
    YP_phonebook_id_t id;
    uuid_generate(id.uuid);

    /* create the new phonebook's context */
    void* context = NULL;
    ret = backend->create_phonebook(provider, in.config, &context);
    if(ret != YP_SUCCESS) {
        out.ret = ret;
        margo_error(provider->mid, "Could not create phonebook, backend returned %d", ret);
        goto finish;
    }

    /* allocate a phonebook, set it up, and add it to the provider */
    YP_phonebook* phonebook = (YP_phonebook*)calloc(1, sizeof(*phonebook));
    phonebook->fn  = backend;
    phonebook->ctx = context;
    phonebook->id  = id;
    add_phonebook(provider, phonebook);

    /* set the response */
    out.ret = YP_SUCCESS;
    out.id = id;

    char id_str[37];
    YP_phonebook_id_to_string(id, id_str);
    margo_debug(provider->mid, "Created phonebook %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_create_phonebook_ult)

static void YP_open_phonebook_ult(hg_handle_t h)
{
    hg_return_t hret;
    YP_return_t ret;
    open_phonebook_in_t  in;
    open_phonebook_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = YP_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    YP_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = YP_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new phonebook */
    YP_phonebook_id_t id;
    uuid_generate(id.uuid);

    /* create the new phonebook's context */
    void* context = NULL;
    ret = backend->open_phonebook(provider, in.config, &context);
    if(ret != YP_SUCCESS) {
        margo_error(mid, "Backend failed to open phonebook");
        out.ret = ret;
        goto finish;
    }

    /* allocate a phonebook, set it up, and add it to the provider */
    YP_phonebook* phonebook = (YP_phonebook*)calloc(1, sizeof(*phonebook));
    phonebook->fn  = backend;
    phonebook->ctx = context;
    phonebook->id  = id;
    add_phonebook(provider, phonebook);

    /* set the response */
    out.ret = YP_SUCCESS;
    out.id = id;

    char id_str[37];
    YP_phonebook_id_to_string(id, id_str);
    margo_debug(mid, "Created phonebook %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_open_phonebook_ult)

static void YP_close_phonebook_ult(hg_handle_t h)
{
    hg_return_t hret;
    YP_return_t ret;
    close_phonebook_in_t  in;
    close_phonebook_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = YP_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* remove the phonebook from the provider
     * (its close function will be called) */
    ret = remove_phonebook(provider, &in.id, 1);
    out.ret = ret;

    char id_str[37];
    YP_phonebook_id_to_string(in.id, id_str);
    margo_debug(mid, "Removed phonebook with id %s", id_str);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_close_phonebook_ult)

static void YP_destroy_phonebook_ult(hg_handle_t h)
{
    hg_return_t hret;
    destroy_phonebook_in_t  in;
    destroy_phonebook_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = YP_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the phonebook */
    YP_phonebook* phonebook = find_phonebook(provider, &in.id);
    if(!phonebook) {
        margo_error(mid, "Could not find phonebook");
        out.ret = YP_ERR_INVALID_PHONEBOOK;
        goto finish;
    }

    /* destroy the phonebook's context */
    phonebook->fn->destroy_phonebook(phonebook->ctx);

    /* remove the phonebook from the provider
     * (its close function will NOT be called) */
    out.ret = remove_phonebook(provider, &in.id, 0);

    if(out.ret == YP_SUCCESS) {
        char id_str[37];
        YP_phonebook_id_to_string(in.id, id_str);
        margo_debug(mid, "Destroyed phonebook with id %s", id_str);
    } else {
        margo_error(mid, "Could not destroy phonebook, phonebook may be left in an invalid state");
    }


finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_destroy_phonebook_ult)

static void YP_list_phonebooks_ult(hg_handle_t h)
{
    hg_return_t hret;
    list_phonebooks_in_t  in;
    list_phonebooks_out_t out;
    out.ids = NULL;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = YP_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* allocate array of phonebook ids */
    out.ret   = YP_SUCCESS;
    out.count = provider->num_phonebooks < in.max_ids ? provider->num_phonebooks : in.max_ids;
    out.ids   = (YP_phonebook_id_t*)calloc(provider->num_phonebooks, sizeof(*out.ids));

    /* iterate over the hash of phonebooks to fill the array of phonebook ids */
    unsigned i = 0;
    YP_phonebook *r, *tmp;
    HASH_ITER(hh, provider->phonebooks, r, tmp) {
        out.ids[i++] = r->id;
    }

    margo_debug(mid, "Listed phonebooks");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    free(out.ids);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_list_phonebooks_ult)

static void YP_hello_ult(hg_handle_t h)
{
    hg_return_t hret;
    hello_in_t in;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        goto finish;
    }

    /* find the phonebook */
    YP_phonebook* phonebook = find_phonebook(provider, &in.phonebook_id);
    if(!phonebook) {
        margo_error(mid, "Could not find requested phonebook");
        goto finish;
    }

    /* call hello on the phonebook's context */
    phonebook->fn->hello(phonebook->ctx);

    margo_debug(mid, "Called hello RPC");

finish:
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_hello_ult)

static void YP_sum_ult(hg_handle_t h)
{
    hg_return_t hret;
    sum_in_t     in;
    sum_out_t   out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    YP_provider_t provider = (YP_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = YP_ERR_FROM_MERCURY;
        goto finish;
    }

    /* find the phonebook */
    YP_phonebook* phonebook = find_phonebook(provider, &in.phonebook_id);
    if(!phonebook) {
        margo_error(mid, "Could not find requested phonebook");
        out.ret = YP_ERR_INVALID_PHONEBOOK;
        goto finish;
    }

    /* call sum on the phonebook's context */
    out.result = phonebook->fn->sum(phonebook->ctx, in.x, in.y);
    out.ret = YP_SUCCESS;

    margo_debug(mid, "Called sum RPC");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(YP_sum_ult)

static inline YP_phonebook* find_phonebook(
        YP_provider_t provider,
        const YP_phonebook_id_t* id)
{
    YP_phonebook* phonebook = NULL;
    HASH_FIND(hh, provider->phonebooks, id, sizeof(YP_phonebook_id_t), phonebook);
    return phonebook;
}

static inline YP_return_t add_phonebook(
        YP_provider_t provider,
        YP_phonebook* phonebook)
{
    YP_phonebook* existing = find_phonebook(provider, &(phonebook->id));
    if(existing) {
        return YP_ERR_INVALID_PHONEBOOK;
    }
    HASH_ADD(hh, provider->phonebooks, id, sizeof(YP_phonebook_id_t), phonebook);
    provider->num_phonebooks += 1;
    return YP_SUCCESS;
}

static inline YP_return_t remove_phonebook(
        YP_provider_t provider,
        const YP_phonebook_id_t* id,
        int close_phonebook)
{
    YP_phonebook* phonebook = find_phonebook(provider, id);
    if(!phonebook) {
        return YP_ERR_INVALID_PHONEBOOK;
    }
    YP_return_t ret = YP_SUCCESS;
    if(close_phonebook) {
        ret = phonebook->fn->close_phonebook(phonebook->ctx);
    }
    HASH_DEL(provider->phonebooks, phonebook);
    free(phonebook);
    provider->num_phonebooks -= 1;
    return ret;
}

static inline void remove_all_phonebooks(
        YP_provider_t provider)
{
    YP_phonebook *r, *tmp;
    HASH_ITER(hh, provider->phonebooks, r, tmp) {
        HASH_DEL(provider->phonebooks, r);
        r->fn->close_phonebook(r->ctx);
        free(r);
    }
    provider->num_phonebooks = 0;
}

static inline YP_backend_impl* find_backend_impl(
        YP_provider_t provider,
        const char* name)
{
    size_t i;
    for(i = 0; i < provider->num_backend_types; i++) {
        YP_backend_impl* impl = provider->backend_types[i];
        if(strcmp(name, impl->name) == 0)
            return impl;
    }
    return NULL;
}

static inline YP_return_t add_backend_impl(
        YP_provider_t provider,
        YP_backend_impl* backend)
{
    provider->num_backend_types += 1;
    provider->backend_types = realloc(provider->backend_types,
                                      provider->num_backend_types);
    provider->backend_types[provider->num_backend_types-1] = backend;
    return YP_SUCCESS;
}

static inline int check_token(
        YP_provider_t provider,
        const char* token)
{
    if(!provider->token) return 1;
    return !strcmp(provider->token, token);
}
