/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_CHANNEL_TESTS_PH
#define D_TSTORAGE_CHANNEL_TESTS_PH

namespace tstorage {

int test_channel_connect();
int test_channel_getacq();
int test_channel_get_empty();
int test_channel_put_empty();
int test_channel_put();
int test_channel_puta();
int test_channel_put_variable_payload_size();
int test_channel_get();
int test_channel_many_records();
int test_channel_large_payloads();
int test_channel_mixed_payloads();
int test_channel_get_with_memory_limit();
int test_channel_get_stream();
int test_channel_put_bad_cid();
int test_channel_put_key_out_of_range();

} /*namespace tstorage*/

#endif
