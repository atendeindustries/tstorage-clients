/*
 * TStorage: Client library (C++)
 *
 * Version.cpp
 *   Access to the current library version information.
 *
 * Copyright 2025 Atende Industries
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tstorageclient++/Version.h>

#include "Defines.h"

#ifndef D_LIBTSCLIENT_VERSION_MAJOR
#define D_LIBTSCLIENT_VERSION_MAJOR -1
#endif

#ifndef D_LIBTSCLIENT_VERSION_MINOR
#define D_LIBTSCLIENT_VERSION_MINOR -1
#endif

#ifndef D_LIBTSCLIENT_VERSION_PATCH
#define D_LIBTSCLIENT_VERSION_PATCH -1
#endif

#ifndef D_LIBTSCLIENT_VERSION
#define D_LIBTSCLIENT_VERSION ???
#endif

/* NOLINTBEGIN(cppcoreguidelines-macro-usage) */
#define STRINGIFY(s) #s
#define STRINGIFY2(s) STRINGIFY(s)
/* NOLINTEND(cppcoreguidelines-macro-usage) */
#define D_LIBTSCLIENT_VERSION_STR STRINGIFY2(D_LIBTSCLIENT_VERSION)

namespace tstorage {

TSTORAGE_EXPORT int client_version_major()
{
	return D_LIBTSCLIENT_VERSION_MAJOR;
}

TSTORAGE_EXPORT int client_version_minor()
{
	return D_LIBTSCLIENT_VERSION_MINOR;
}

TSTORAGE_EXPORT int client_version_patch()
{
	return D_LIBTSCLIENT_VERSION_PATCH;
}

TSTORAGE_EXPORT const char* client_version_string()
{
	return D_LIBTSCLIENT_VERSION_STR;
}

TSTORAGE_EXPORT const char* client_name() {
	return "libtsclient++ v" D_LIBTSCLIENT_VERSION_STR;
}

} /*namespace tstorage*/
