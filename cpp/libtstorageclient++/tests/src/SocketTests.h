/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_SOCKET_TESTS_PH
#define D_TSTORAGE_SOCKET_TESTS_PH

namespace tstorage {

int test_socket_close();
int test_socket_send();
int test_socket_recv();
int test_socket_recv_exactly();
int test_socket_recv_atleast();
int test_socket_skip_exactly();
int test_socket_shutdown_send();
int test_socket_shutdown_recv();
int test_socket_dialog();
int test_socket_send_timeout();
int test_socket_recv_timeout();

} /*namespace tstorage*/

#endif
