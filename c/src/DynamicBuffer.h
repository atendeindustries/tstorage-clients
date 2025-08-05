/*
 * Copyright 2025 Atende Industries sp. z o.o.
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
 *
 */

/** @file DynamicBuffer.h
 *
 * Dynamic buffer data structure - public interface.
 */

#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include <stddef.h>

#include "ErrorCode.h"

/**
 * @brief A dynamic buffer data structure.
 *
 * It grows with amortized constant cost - see DynamicBuffer_resize.
 */
typedef struct DynamicBuffer
{
	/* Maximum possible size of the storage. */
	size_t maxSize;
	/* Current size of the storage. */
	size_t size;
	/* Storage for the buffer's contents. */
	char* storage;
} DynamicBuffer;

/**
 * @brief Initializes the buffer with given parameters.
 *
 * @param this The buffer being initialized.
 * @param maxSize Maximum size of the buffer.
 * @param size Initial size of the buffer. Must be <= 'maxSize'.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 *
 * @public @memberof DynamicBuffer
 */
ErrorCode DynamicBuffer_initialize(DynamicBuffer* this, size_t maxSize, size_t size);

/**
 * @brief Frees the buffer previously initialized.
 *
 * @param this The buffer being finalized.
 *
 * @public @memberof DynamicBuffer
 */
void DynamicBuffer_finalize(DynamicBuffer* this);

/**
 * @brief Changes the buffer's maximum size.
 *
 * Also empties the buffer, i.e. resets its size to 0.
 *
 * @param this The buffer being changed.
 *
 * @public @memberof DynamicBuffer
 */
void DynamicBuffer_setMaxSize(DynamicBuffer* this, size_t size);

/**
 * @brief Increases the buffer's size to at least the given value.
 *
 * If one were to call DynamicBuffer_resize increasing the 'size' argument by
 * 1 on each call, then the amortized cost of all calls would be constant.
 * This is achieved by this function setting the size larger than requested.
 *
 * @param this The buffer being resized.
 * @param capacity The buffer's new minimum size.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_LIMIT if 'size' is greater than 'this''s maximum size.
 *
 * @public @memberof DynamicArray
 */
ErrorCode DynamicBuffer_resize(DynamicBuffer* this, size_t size);

#endif /* DYNAMICBUFFER_H */
