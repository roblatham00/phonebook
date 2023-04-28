/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <alpha/alpha-server.h>
#include <alpha/alpha-admin.h>
#include <alpha/alpha-client.h>
#include <alpha/alpha-resource.h>

struct test_context {
    margo_instance_id   mid;
    hg_addr_t           addr;
    alpha_admin_t       admin;
    alpha_resource_id_t id;
};

static const char* token = "ABCDEFGH";
static const uint16_t provider_id = 42;
static const char* backend_config = "{ \"foo\" : \"bar\" }";

TEST_CASE("Test client interface", "[client]") {

    alpha_return_t      ret;
    margo_instance_id   mid;
    hg_addr_t           addr;
    alpha_admin_t       admin;
    alpha_resource_id_t id;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    REQUIRE(mid != MARGO_INSTANCE_NULL);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    REQUIRE(hret == HG_SUCCESS);
    // register alpha provider
    struct alpha_provider_args args = ALPHA_PROVIDER_ARGS_INIT;
    args.token = token;
    ret = alpha_provider_register(
            mid, provider_id, &args,
            ALPHA_PROVIDER_IGNORE);
    REQUIRE(ret == ALPHA_SUCCESS);
    // create an admin
    ret = alpha_admin_init(mid, &admin);
    REQUIRE(ret == ALPHA_SUCCESS);
    // create a resource using the admin
    ret = alpha_create_resource(admin, addr,
            provider_id, token, "dummy", backend_config, &id);
    REQUIRE(ret == ALPHA_SUCCESS);
    // create test context
    auto context = std::make_unique<test_context>();
    context->mid   = mid;
    context->addr  = addr;
    context->admin = admin;
    context->id    = id;

    SECTION("Create client") {
        alpha_client_t client;
        alpha_return_t ret;
        // test that we can create a client object
        ret = alpha_client_init(context->mid, &client);
        REQUIRE(ret == ALPHA_SUCCESS);

        SECTION("Open resource") {
            alpha_resource_handle_t rh;
            // test that we can create a resource handle
            ret = alpha_resource_handle_create(client,
                    context->addr, provider_id, context->id, &rh);
            REQUIRE(ret == ALPHA_SUCCESS);

            SECTION("Send hello RPC") {
                // test that we can send a hello RPC to the resource
                ret = alpha_say_hello(rh);
                REQUIRE(ret == ALPHA_SUCCESS);
            }

            SECTION("Send sum RPC") {
                // test that we can send a sum RPC to the resource
                int32_t result = 0;
                ret = alpha_compute_sum(rh, 45, 55, &result);
                REQUIRE(ret == ALPHA_SUCCESS);
                REQUIRE(result == 100);
            }

            // test that we can increase the ref count
            ret = alpha_resource_handle_ref_incr(rh);
            REQUIRE(ret == ALPHA_SUCCESS);
            // test that we can destroy the resource handle
            ret = alpha_resource_handle_release(rh);
            REQUIRE(ret == ALPHA_SUCCESS);
            // ... and a second time because of the increase ref
            ret = alpha_resource_handle_release(rh);
            REQUIRE(ret == ALPHA_SUCCESS);
        }

        SECTION("Invalid calls") {
            alpha_resource_handle_t rh1, rh2;
            alpha_resource_id_t invalid_id;
            // create a resource handle for a wrong resource id
            ret = alpha_resource_handle_create(client,
                    context->addr, provider_id, invalid_id, &rh1);
            REQUIRE(ret == ALPHA_SUCCESS);
            // create a resource handle for a wrong provider id
            ret = alpha_resource_handle_create(client,
                    context->addr, provider_id + 1, context->id, &rh2);
            REQUIRE(ret == ALPHA_SUCCESS);
            // test sending to the invalid resource id
            int32_t result;
            ret = alpha_compute_sum(rh1, 45, 55, &result);
            REQUIRE(ret == ALPHA_ERR_INVALID_RESOURCE);
            // test sending to the invalid provider id
            ret = alpha_compute_sum(rh2, 45, 55, &result);
            REQUIRE(ret == ALPHA_ERR_FROM_MERCURY);
            // test that we can destroy the resource handle
            ret = alpha_resource_handle_release(rh1);
            REQUIRE(ret == ALPHA_SUCCESS);
            // test that we can destroy the resource handle
            ret = alpha_resource_handle_release(rh2);
            REQUIRE(ret == ALPHA_SUCCESS);
        }

        // test that we can free the client object
        ret = alpha_client_finalize(client);
        REQUIRE(ret == ALPHA_SUCCESS);
    }

    // destroy the resource
    ret = alpha_destroy_resource(context->admin,
            context->addr, provider_id, token, context->id);
    REQUIRE(ret == ALPHA_SUCCESS);
    // free the admin
    ret = alpha_admin_finalize(context->admin);
    REQUIRE(ret == ALPHA_SUCCESS);
    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}
