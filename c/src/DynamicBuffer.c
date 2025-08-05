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

/** @file DynamicBuffer.c
 *
 * Dynamic buffer data structure - implementation.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "DynamicBuffer.h"

#include "ErrorCode.h"

/**
 * @brief Sets the buffer's exact size (grows or shrinks).
 *
 * @param this the buffer being resized
 * @param size the new size. Must be <= this->maxSize. The buffer will be
 * reallocated even if 'size' equals the current size.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3). Will not happen when
 * 'size' is 0.
 */
static ErrorCode setSize(DynamicBuffer* this, size_t size)
{
	if (size == 0) {
		this->size = 0;
		free(this->storage);
		this->storage = NULL;
		return ERR_OK;
	}

	assert(size <= this->maxSize);

	char* storage = realloc(this->storage, size);
	if (storage == NULL) {
		return ERR_RESOURCE;
	}

	this->size = size;
	this->storage = storage;

	return 0;
}


ErrorCode DynamicBuffer_initialize(DynamicBuffer* this, size_t maxSize, size_t size)
{
	assert(size <= maxSize);

	this->maxSize = maxSize;
	this->storage = NULL;

	return setSize(this, size);
}

void DynamicBuffer_finalize(DynamicBuffer* this)
{
	free(this->storage);
}

void DynamicBuffer_setMaxSize(DynamicBuffer* this, size_t size)
{
	ErrorCode setRes = setSize(this, 0);
	assert(setRes == ERR_OK);

	this->maxSize = size;
}

/**
 * @brief Grows the buffer's size to at least 'size'.
 *
 * The size may grow greater than requested, to satisfy constant amortized
 * cost of repeatedly growing by one.
 *
 * @param this The buffer being grown.
 * @param size The new size. It is > this->size.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_LIMIT if the buffer's size already reached maximum size.
 */
static ErrorCode grow(DynamicBuffer* this, size_t size)
{
	assert(size > this->size);

	if (size > this->maxSize) {
		return ERR_LIMIT;
	}

	/* Grow at least 'growMult' times, or to exactly 'size' whichever is
	   greater. But never grow more than to 'maxSize'. */
	static const size_t growMult = 2;

	if (this->size > size / growMult) {
		size = this->size > this->maxSize / growMult ? this->maxSize
													 : this->size * growMult;
	}

	return setSize(this, size);
}

ErrorCode DynamicBuffer_resize(DynamicBuffer* this, size_t size)
{
	if (size > this->size) {
		return grow(this, size);
	}
	return ERR_OK;
}
