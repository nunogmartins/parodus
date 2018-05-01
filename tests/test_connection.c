/**
 *  Copyright 2010-2016 Comcast Cable Communications Management, LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <stdarg.h>

#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <assert.h>
#include <CUnit/Basic.h>
#include <nopoll.h>

#include "../src/ParodusInternal.h"
#include "../src/connection.h"
#include "../src/config.h"

extern void set_server_null (server_t *server);
extern void set_server_list_null (server_list_t *server_list);
extern int server_is_null (server_t *server);
extern server_t *get_current_server (server_list_t *server_list);
extern int parse_server_url (const char *full_url, server_t *server);

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/

bool close_retry;
bool LastReasonStatus;
volatile unsigned int heartBeatTimer;
pthread_mutex_t close_mut;

/*----------------------------------------------------------------------------*/
/*                                   Mocks                                    */
/*----------------------------------------------------------------------------*/
char* getWebpaConveyHeader()
{
    return NULL;
}

int checkHostIp(char * serverIP)
{
    UNUSED(serverIP);
    return 0;
}

void setMessageHandlers()
{
}
/*----------------------------------------------------------------------------*/
/*                                   Tests                                    */
/*----------------------------------------------------------------------------*/

void test_get_global_conn()
{
    assert_null(get_global_conn());
}

void test_set_global_conn()
{
    static noPollConn *gNPConn;
    set_global_conn(gNPConn);
    assert_ptr_equal(gNPConn, get_global_conn());
}

void test_get_global_reconnect_reason()
{
    assert_string_equal("webpa_process_starts", get_global_reconnect_reason());
}

void test_set_global_reconnect_reason()
{
    char *reason = "Factory-Reset";
    set_global_reconnect_reason(reason);
    assert_string_equal(reason, get_global_reconnect_reason());
}

void test_closeConnection()
{
    close_and_unref_connection(get_global_conn());
}

void test_server_is_null()
{
  server_t test_server;
  memset (&test_server, 0xFF, sizeof(test_server));
  assert_int_equal (0, server_is_null (&test_server));
  set_server_null (&test_server);
  assert_int_equal (1, server_is_null (&test_server));
}

void test_server_list_null()
{
  server_list_t server_list;
  memset (&server_list, 0xFF, sizeof(server_list));
  assert_int_equal (0, server_is_null (&server_list.defaults));
  assert_int_equal (0, server_is_null (&server_list.jwt));
  assert_int_equal (0, server_is_null (&server_list.redirect));
  set_server_list_null (&server_list);
  assert_int_equal (1, server_is_null (&server_list.defaults));
  assert_int_equal (1, server_is_null (&server_list.jwt));
  assert_int_equal (1, server_is_null (&server_list.redirect));
 
}

void test_get_current_server()
{
  server_list_t server_list;
  memset (&server_list, 0xFF, sizeof(server_list));
  assert_ptr_equal (&server_list.redirect, get_current_server (&server_list));
  set_server_null (&server_list.redirect);
  assert_ptr_equal (&server_list.jwt, get_current_server (&server_list));
  set_server_null (&server_list.jwt);
  assert_ptr_equal (&server_list.defaults, get_current_server (&server_list));
}

void test_parse_server_url ()
{
	server_t test_server;
	assert_int_equal (parse_server_url ("mydns.mycom.net:8080",
		&test_server), -1);
	assert_int_equal (-1, test_server.allow_insecure);
	assert_int_equal (parse_server_url ("https://mydns.mycom.net:8080",
		&test_server), 0);
	assert_string_equal (test_server.server_addr, "mydns.mycom.net");
	assert_string_equal (test_server.port, "8080");
	assert_int_equal (0, test_server.allow_insecure);
	assert_int_equal (parse_server_url ("https://mydns.mycom.net/",
		&test_server), 0);
	assert_string_equal (test_server.server_addr, "mydns.mycom.net");
	assert_string_equal (test_server.port, "443");
	assert_int_equal (0, test_server.allow_insecure);
	assert_int_equal (parse_server_url ("http://mydns.mycom.net:8080",
		&test_server), 1);
	assert_string_equal (test_server.server_addr, "mydns.mycom.net");
	assert_string_equal (test_server.port, "8080");
	assert_int_equal (1, test_server.allow_insecure);
	assert_int_equal (parse_server_url ("http://mydns.mycom.net",
		&test_server), 1);
	assert_string_equal (test_server.server_addr, "mydns.mycom.net");
	assert_string_equal (test_server.port, "80");
	assert_int_equal (1, test_server.allow_insecure);
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_get_global_conn),
        cmocka_unit_test(test_set_global_conn),
        cmocka_unit_test(test_get_global_reconnect_reason),
        cmocka_unit_test(test_set_global_reconnect_reason),
        cmocka_unit_test(test_closeConnection),
        cmocka_unit_test(test_server_is_null),
        cmocka_unit_test(test_server_list_null),
        cmocka_unit_test(test_get_current_server),
        cmocka_unit_test(test_parse_server_url)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
