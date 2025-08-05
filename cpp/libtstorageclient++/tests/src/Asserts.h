/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_ASSERTS_PH
#define D_TSTORAGE_ASSERTS_PH

#include <cmath>
#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

namespace tstorage {

void print_mem(const void* x, std::size_t size);

/* NOLINTBEGIN(cppcoreguidelines-macro-usage) */
#define STRAUX(x) #x
#define STR(x) STRAUX(x)
#define ASSERT_EQ(x, expected) \
	if ((x) != (expected)) { \
		cout << "[ERROR] Assert failed: " #x " != " << (expected) \
			 << " @ line: " STR(__LINE__) " (got " << (x) << ")" << endl; \
		return 1; \
	}

#define ASSERT_NEQ(x, expected) \
	if ((x) == (expected)) { \
		cout << "[ERROR] Assert failed: " #x " != " << (expected) \
			 << " @ line: " STR(__LINE__) " (got " << (x) << ")" << endl; \
		return 1; \
	}

#define ASSERT_NULL(x) \
	if ((x) != nullptr) { \
		cout << "[ERROR] Assert failed: " #x " == nullptr" \
			 << " @ line: " STR(__LINE__) " (got " << (x) << ")" << endl; \
		return 1; \
	}

#define ASSERT_NON_NULL(x) \
	if ((x) == nullptr) { \
		cout << "[ERROR] Assert failed: " #x " != nullptr" \
			 << " @ line: " STR(__LINE__) << endl; \
		return 1; \
	}

#define ASSERT_LEQ(x, expected) \
	if ((x) > (expected)) { \
		cout << "[ERROR] Assert failed: " #x " <= " << (expected) \
			 << " @ line: " STR(__LINE__) " (got " << (x) << ")" << endl; \
		return 1; \
	}

#define ASSERT_GEQ(x, expected) \
	if ((x) < (expected)) { \
		cout << "[ERROR] Assert failed: " #x " >= " << (expected) \
			 << " @ line: " STR(__LINE__) " (got " << (x) << ")" << endl; \
		return 1; \
	}

#define ASSERT_MEM_EQ(x, expected, size) \
	if (memcmp((x), (expected), (size)) != 0) { \
		cout << "[ERROR] Assert failed: " #x " != "; \
		print_mem((expected), (size)); \
		cout << " @ line: " STR(__LINE__) " (got "; \
		print_mem((x), (size)); \
		cout << ")" << endl; \
		return 1; \
	}

#define LOG(x) \
	cout << #x " = " << (x) << " @ line: " STR(__LINE__) << endl;
/* NOLINTEND(cppcoreguidelines-macro-usage) */

} /*namespace tstorage*/

#endif
