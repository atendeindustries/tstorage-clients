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

/** @file DynamicArray.c
 *
 * Dynamic array data structure - implementation.
 */

#include <assert.h>
#include <stddef.h>

#include "DynamicArray.h"

#include "DynamicBuffer.h"
#include "ErrorCode.h"

ErrorCode DynamicArray_initialize(DynamicArray* this, size_t elemSize, size_t maxCapacity, size_t size)
{
	assert(elemSize > 0);
	assert(maxCapacity <= SIZE_MAX / elemSize
		   && "Size in bytes would exceed system limitations");

	this->elemSize = elemSize;
	this->size = size;
	return DynamicBuffer_initialize(&this->buffer, maxCapacity * elemSize, size * elemSize);
}

void DynamicArray_finalize(DynamicArray* this)
{
	DynamicBuffer_finalize(&this->buffer);
}

/**
 * @brief Computes the array's current capacity, i.e. number of elements that
 * the array's dynamic buffer can currently fit.
 *
 * @param this The array being investigated.
 * @return 'this''s capacity.
 */
static size_t capacity(const DynamicArray* this)
{
	return this->buffer.size / this->elemSize;
}

/**
 * @brief Increases the array's capacity to at least the given value.
 *
 * The capacity may grow greater than requested, to satisfy
 * constant amortized cost of repeatedly growing by one.
 *
 * @param this The array being resized.
 * @param capacity The array's new minimum capacity.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_LIMIT if 'capacity' is greater than 'this''s maximum capacity.
 *
 * @public @memberof DynamicArray
 */
static ErrorCode reserve(DynamicArray* this, size_t capacity)
{
	if (capacity > SIZE_MAX / this->elemSize) {
		return ERR_LIMIT;
	}

	return DynamicBuffer_resize(&this->buffer, capacity * this->elemSize);
}

ErrorCode DynamicArray_emplaceBack(DynamicArray* restrict this, void* restrict* restrict elem)
{
	if (this->size == SIZE_MAX) {
		return ERR_LIMIT;
	}
	ErrorCode res = reserve(this, this->size + 1);
	if (res != ERR_OK) {
		return res;
	}
	assert(this->size <= capacity(this));

	*elem = this->buffer.storage + this->size++ * this->elemSize;
	return ERR_OK;
}

void DynamicArray_removeLast(DynamicArray* this)
{
	assert(this->size > 0 && "Removal from an empty array");
	--this->size;
}

void DynamicArray_empty(DynamicArray* this)
{
	this->size = 0;
}
