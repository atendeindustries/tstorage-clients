/*
 * TStorage: Client library (C++)
 *
 * Defines.h
 *   Some platform-specific pragmas and attribute definitions.
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

#ifndef D_TSTORAGE_DEFINES_PH
#define D_TSTORAGE_DEFINES_PH

/** @file
 * @brief Provides various compile-time switches and symbol export macros. */

/** @def TSTORAGE_EXPORT
 * @brief A symbol export attribute macro. */

#ifndef DEBUG

#if defined(__GNUC__) || defined(__clang__)
#define TSTORAGE_EXPORT [[gnu::visibility("default")]]
#elif defined(_MSC_VER)
#define TSTORAGE_EXPORT __declspec(dllexport)
#else
#define TSTORAGE_EXPORT __attribute__((visibility("hidden")))
#endif

#else

#define TSTORAGE_EXPORT

#endif


#endif
