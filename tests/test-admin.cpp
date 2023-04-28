/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <stdio.h>
#include <margo.h>
#include <alpha/alpha-server.h>
#include <alpha/alpha-admin.h>

struct test_context {
    margo_instance_id mid;
    hg_addr_t         addr;
};

static const char* valid_token = "ABCDEFGH";
static const char* wrong_token = "HGFEDCBA";
static const uint16_t provider_id = 42;
static const char* backend_config = "{ \"foo\" : \"bar\" }";

TEST_CASE("Test admin interface", "[admin]") {
    margo_instance_id mid;
    hg_addr_t         addr;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    REQUIRE(mid != MARGO_INSTANCE_NULL);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    REQUIRE(hret == HG_SUCCESS);
    // register alpha provider
    struct alpha_provider_args args = ALPHA_PROVIDER_ARGS_INIT;
    args.token = valid_token;
    alpha_return_t ret = alpha_provider_register(
            mid, provider_id, &args,
            ALPHA_PROVIDER_IGNORE);
    REQUIRE(ret == ALPHA_SUCCESS);
    // create test context
    auto context = std::make_unique<test_context>();
    context->mid  = mid;
    context->addr = addr;

    SECTION("Admin initialization") {
        alpha_admin_t admin;
        alpha_return_t ret;
        // test that we can create an admin object
        ret = alpha_admin_init(context->mid, &admin);
        REQUIRE(ret == ALPHA_SUCCESS);
        // test that we can free the admin object
        ret = alpha_admin_finalize(admin);
        REQUIRE(ret == ALPHA_SUCCESS);
    }

    SECTION("Resource access") {
        alpha_admin_t admin;
        alpha_return_t ret;
        alpha_resource_id_t id;
        // test that we can create an admin object
        ret = alpha_admin_init(context->mid, &admin);
        REQUIRE(ret == ALPHA_SUCCESS);

        // test that we can create a resource with type "dummy"
        ret = alpha_create_resource(admin, context->addr,
                provider_id, valid_token, "dummy", backend_config, &id);
        REQUIRE(ret == ALPHA_SUCCESS);

        // test that we can list the resources
        alpha_resource_id_t ids[4];
        size_t count = 4;
        ret = alpha_list_resources(admin, context->addr,
                provider_id, valid_token, ids, &count);
        REQUIRE(ret == ALPHA_SUCCESS);
        REQUIRE(count == 1);
        REQUIRE(memcmp(&ids[0], &id, sizeof(id)) == 0);

        // test that we can destroy the resource we just created
        ret = alpha_destroy_resource(admin, context->addr,
                provider_id, valid_token, id);
        REQUIRE(ret == ALPHA_SUCCESS);
        // note: open and close are essentially the same as create and
        // destroy in this code so we won't be testing them.

        // test that we can free the admin object
        ret = alpha_admin_finalize(admin);
        REQUIRE(ret == ALPHA_SUCCESS);
    }

    SECTION("Invalid resource accesses") {
        alpha_admin_t admin;
        alpha_return_t ret;
        alpha_resource_id_t id;
        // test that we can create an admin object
        ret = alpha_admin_init(context->mid, &admin);
        REQUIRE(ret == ALPHA_SUCCESS);

        // test that calling the wrong provider id leads to an error
        ret = alpha_create_resource(admin, context->addr,
                provider_id + 1, valid_token, "dummy", backend_config, &id);
        REQUIRE(ret == ALPHA_ERR_FROM_MERCURY);

        // test that calling with the wrong token leads to an error
        ret = alpha_create_resource(admin, context->addr,
                provider_id, wrong_token, "dummy", backend_config, &id);
        REQUIRE(ret == ALPHA_ERR_INVALID_TOKEN);

        // test that calling with the wrong config leads to an error
        ret = alpha_create_resource(admin, context->addr,
                provider_id, valid_token, "dummy", "{ashqw{", &id);
        REQUIRE(ret == ALPHA_ERR_INVALID_CONFIG);

        // test that calling with an unknown backend leads to an error
        ret = alpha_create_resource(admin, context->addr,
                provider_id, valid_token, "blah", backend_config, &id);
        REQUIRE(ret == ALPHA_ERR_INVALID_BACKEND);

        // this creation should be successful
        ret = alpha_create_resource(admin, context->addr,
                provider_id, valid_token, "dummy", backend_config, &id);
        REQUIRE(ret == ALPHA_SUCCESS);

        // test that destroying an invalid id leads to an error
        alpha_resource_id_t wrong_id;
        memset((void*) &wrong_id, 0, sizeof(wrong_id));
        ret = alpha_destroy_resource(admin, context->addr, provider_id, valid_token, wrong_id);
        REQUIRE(ret == ALPHA_ERR_INVALID_RESOURCE);

        // correctly destroy the created resource
        ret = alpha_destroy_resource(admin, context->addr, provider_id, valid_token, id);
        REQUIRE(ret == ALPHA_SUCCESS);

        // test that we can free the admin object
        ret = alpha_admin_finalize(admin);
        REQUIRE(ret == ALPHA_SUCCESS);
    }

    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}
