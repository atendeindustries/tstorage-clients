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

/** @file DynamicArray.h
 *
 * Dynamic array data structure - public interface.
 */

#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <stddef.h>

#include "DynamicBuffer.h"
#include "ErrorCode.h"

/**
 * @brief A dynamic array data structure.
 *
 * It expands with amortized constant cost.
 */
typedef struct DynamicArray
{
	DynamicBuffer buffer;
	/* Current number of elements in the array. */
	size_t size;
	/* Size of a single array element, in bytes. */
	size_t elemSize;
} DynamicArray;

/**
 * @brief Initializes the array structure.
 *
 * @param this The array being initialized.
 * @param elemSize Size of a single array element, in bytes. Must be > 0.
 * @param maxCapacity Maximum number of elements. maxCapacity * elemMax
 * must not exceed SIZE_MAX. For "unlimited" capacity, set maxCapacity to
 * SIZE_MAX / elemSize.
 * @param size Initial size. Must be <= 'maxCapacity'.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 *
 * @public @memberof DynamicArray
 */
ErrorCode DynamicArray_initialize(DynamicArray* this, size_t elemSize, size_t maxCapacity, size_t size);

/**
 * @brief Frees the array previously initialized.
 *
 * @param this The array being finalized.
 *
 * @public @memberof DynamicArray
 */
void DynamicArray_finalize(DynamicArray* this);

/**
 * @brief Allocates space for a single element at the end.
 *
 * @param this The array being added to.
 * @param[out] elem Pointer to the new element's storage area.
 * @return ERR_OK on success.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_LIMIT if the array's size already reached maximum capacity.
 *
 * @public @memberof DynamicArray
 */
ErrorCode DynamicArray_emplaceBack(DynamicArray* restrict this, void* restrict* restrict elem);

/**
 * @brief Shortens the aray by 1. Does not change capacity.
 *
 * @param this The array being added to. Must ot be empty.
 * @param[out] elem Pointer to the new element's storage area.
 *
 * @public @memberof DynamicArray
 */
void DynamicArray_removeLast(DynamicArray* this);

/**
 * @brief Removes all elements from an array. Does not change capacity.
 *
 * @param this The array being emptied
 *
 * @public @memberof DynamicArray
 */
void DynamicArray_empty(DynamicArray* this);

#endif /* DYNAMICARRAY_H */
