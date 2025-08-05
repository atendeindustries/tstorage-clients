/*
 * TStorage: Client library (C++)
 *
 * Version.h
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

#ifndef D_TSTORAGE_VERSION_H
#define D_TSTORAGE_VERSION_H

/** @file
 * @brief Provides access to version information of the library. */

namespace tstorage {

/** @brief Returns the major version number of the linked libtstorageclient++
 * library. */
int client_version_major();

/** @brief Returns the minor version number of the linked libtstorageclient++
 * library. */
int client_version_minor();

/** @brief Returns the patch number of the linked libtstorageclient++ library. */
int client_version_patch();

/** @brief Returns version string of the linked libtstorageclient++ library. */
const char* client_version_string();

/** @brief Returns the name of the linked libtstorageclient++ library. */
const char* client_name();

} /*namespace tstorage*/

#endif
