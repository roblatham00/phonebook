/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <YP/YP-server.h>
#include <YP/YP-admin.h>
#include <YP/YP-client.h>
#include <YP/YP-phonebook.h>

struct test_context {
    margo_instance_id   mid;
    hg_addr_t           addr;
    YP_admin_t       admin;
    YP_phonebook_id_t id;
};

static const char* token = "ABCDEFGH";
static const uint16_t provider_id = 42;
static const char* backend_config = "{ \"foo\" : \"bar\" }";

TEST_CASE("Test client interface", "[client]") {

    YP_return_t      ret;
    margo_instance_id   mid;
    hg_addr_t           addr;
    YP_admin_t       admin;
    YP_phonebook_id_t id;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    REQUIRE(mid != MARGO_INSTANCE_NULL);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    REQUIRE(hret == HG_SUCCESS);
    // register YP provider
    struct YP_provider_args args = YP_PROVIDER_ARGS_INIT;
    args.token = token;
    ret = YP_provider_register(
            mid, provider_id, &args,
            YP_PROVIDER_IGNORE);
    REQUIRE(ret == YP_SUCCESS);
    // create an admin
    ret = YP_admin_init(mid, &admin);
    REQUIRE(ret == YP_SUCCESS);
    // create a phonebook using the admin
    ret = YP_create_phonebook(admin, addr,
            provider_id, token, "dummy", backend_config, &id);
    REQUIRE(ret == YP_SUCCESS);
    // create test context
    auto context = std::make_unique<test_context>();
    context->mid   = mid;
    context->addr  = addr;
    context->admin = admin;
    context->id    = id;

    SECTION("Create client") {
        YP_client_t client;
        YP_return_t ret;
        // test that we can create a client object
        ret = YP_client_init(context->mid, &client);
        REQUIRE(ret == YP_SUCCESS);

        SECTION("Open phonebook") {
            YP_phonebook_handle_t rh;
            // test that we can create a phonebook handle
            ret = YP_phonebook_handle_create(client,
                    context->addr, provider_id, context->id, &rh);
            REQUIRE(ret == YP_SUCCESS);

            SECTION("Send hello RPC") {
                // test that we can send a hello RPC to the phonebook
                ret = YP_say_hello(rh);
                REQUIRE(ret == YP_SUCCESS);
            }

            SECTION("Send sum RPC") {
                // test that we can send a sum RPC to the phonebook
                int32_t result = 0;
                ret = YP_compute_sum(rh, 45, 55, &result);
                REQUIRE(ret == YP_SUCCESS);
                REQUIRE(result == 100);
            }

            // test that we can increase the ref count
            ret = YP_phonebook_handle_ref_incr(rh);
            REQUIRE(ret == YP_SUCCESS);
            // test that we can destroy the phonebook handle
            ret = YP_phonebook_handle_release(rh);
            REQUIRE(ret == YP_SUCCESS);
            // ... and a second time because of the increase ref
            ret = YP_phonebook_handle_release(rh);
            REQUIRE(ret == YP_SUCCESS);
        }

        SECTION("Invalid calls") {
            YP_phonebook_handle_t rh1, rh2;
            YP_phonebook_id_t invalid_id;
            // create a phonebook handle for a wrong phonebook id
            ret = YP_phonebook_handle_create(client,
                    context->addr, provider_id, invalid_id, &rh1);
            REQUIRE(ret == YP_SUCCESS);
            // create a phonebook handle for a wrong provider id
            ret = YP_phonebook_handle_create(client,
                    context->addr, provider_id + 1, context->id, &rh2);
            REQUIRE(ret == YP_SUCCESS);
            // test sending to the invalid phonebook id
            int32_t result;
            ret = YP_compute_sum(rh1, 45, 55, &result);
            REQUIRE(ret == YP_ERR_INVALID_PHONEBOOK);
            // test sending to the invalid provider id
            ret = YP_compute_sum(rh2, 45, 55, &result);
            REQUIRE(ret == YP_ERR_FROM_MERCURY);
            // test that we can destroy the phonebook handle
            ret = YP_phonebook_handle_release(rh1);
            REQUIRE(ret == YP_SUCCESS);
            // test that we can destroy the phonebook handle
            ret = YP_phonebook_handle_release(rh2);
            REQUIRE(ret == YP_SUCCESS);
        }

        // test that we can free the client object
        ret = YP_client_finalize(client);
        REQUIRE(ret == YP_SUCCESS);
    }

    // destroy the phonebook
    ret = YP_destroy_phonebook(context->admin,
            context->addr, provider_id, token, context->id);
    REQUIRE(ret == YP_SUCCESS);
    // free the admin
    ret = YP_admin_finalize(context->admin);
    REQUIRE(ret == YP_SUCCESS);
    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}
