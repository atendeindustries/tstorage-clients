/*
 * TStorage: Client library (C++)
 *
 * RecordsSet.h
 *   A definition of a record container data type used with `Channel<T>`.
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

#ifndef D_TSTORAGE_RECORDSSET_H
#define D_TSTORAGE_RECORDSSET_H

#include <cstddef>
#include <vector>

#include "DataTypes.h"

/** @file
 * @brief Defines a generic records container. */

namespace tstorage {

/**
 * @brief A simple container for inbound and outbound records.
 *
 * A default-constructible, copyable and moveable container for records with
 * payload of type T. It supports appending and iterating over records only.
 * Use it to pass records to TStorage over an open `Channel`. It is also
 * returned as a part of the response to a GET query.
 *
 * @tparam T Payload type of stored records.
 */
template<typename T>
class RecordsSet final
{
private:
	/**
	 * @brief Internal container implementation. It may be subject to change in
	 * the future, consider it an implementation detail.
	 */
	using Container = std::vector<Record<T>>;

public:
	/**
	 * @brief Internal iterator type.
	 *
	 * Satisfies the mutable LegacyInputIterator named requirement (see the C++
	 * reference for the Iterator library).
	 */
	using iterator = typename Container::const_iterator;
	/**
	 * @brief Internal iterator over constants type.
	 *
	 * Satisfies the immutable LegacyInputIterator named requirement (see the C++
	 * reference for the Iterator library).
	 */
	using const_iterator = typename Container::const_iterator;

	/**
	 * @brief Returns an iterator pointing at the first record of the set.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	iterator begin() { return mRecords.begin(); }
	/**
	 * @brief Returns an iterator pointing at the after-the-end element of the
	 * set. This iterator is non-deferencible and is used for out-of-bounds
	 * checks during iteration.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	iterator end() { return mRecords.end(); }
	/**
	 * @brief Returns a const iterator pointing at the first record of the set.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	const_iterator begin() const { return mRecords.begin(); }
	/**
	 * @brief Returns a const iterator pointing at the after-the-end element of
	 * the set. This iterator is non-deferencible and is used for out-of-bounds
	 * checks during iteration.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	const_iterator end() const { return mRecords.end(); }
	/**
	 * @brief Explicitly returns a const iterator pointing at the first record of
	 * the set.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	const_iterator cbegin() const { return mRecords.cbegin(); }
	/**
	 * @brief Explicitly returns a const iterator pointing at the after-the-end
	 * element of the set. This iterator is non-deferencible and is used for
	 * out-of-bounds checks during iteration.
	 *
	 * Note: an iterator returned from this and other methods might get
	 * invalidated after calling `append()`.
	 */
	const_iterator cend() const { return mRecords.cend(); }

	/**
	 * @brief Appends a record to the end of the container. The record is
	 * constructed in-place using the forwarded argumets.
	 *
	 * @param args Arguments to the constructor of a new record.
	 * @tparam U A template parameter pack with types of the arguments passed to
	 * the constructor of a new record.
	 */
	template<typename... U>
	void append(U&&... args)
	{
		this->mRecords.emplace_back(std::forward<U>(args)...);
	}

	/**
	 * @brief Returns the number of records currently stored in the container.
	 */
	std::size_t size() const { return mRecords.size(); }

private:
	/** @brief The actual container. */
	Container mRecords;
};

} /*namespace tstorage*/

#endif
