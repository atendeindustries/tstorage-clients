/*
 * TStorage: Client library (C++)
 *
 * PayloadType.h
 *   A definition of the custom data serializer interface.
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

#ifndef D_TSTORAGE_PAYLOADTYPE_H
#define D_TSTORAGE_PAYLOADTYPE_H

#include <cstddef>

/** @file
 * @brief Specifies the serialization protocol. */

namespace tstorage {

/**
 * @brief Abstract base class for payload serialization and deserialization.
 *
 * Classes implementing this interface provide serialization and
 * deserialization methods for objects of a given type `T`. They are used for
 * bi-directional communication with TStorage instances over TCP via the
 * `Channel` class.
 *
 * There are no requirements on binary representation of user data stored in
 * the database (endianness included). This means that the user is fully
 * responsible for their own data format and has to implement the corresponding
 * serialization and deserialization methods. It is, however, advisable to have
 * quick access to the actual size of the payload, since this information is
 * reused at multiple points in a single call to a GET/PUT command.
 *
 * Despite there being no requirement on the binary format of the payload,
 * there are, however, some requirements on each method's behaviour with
 * respect to the arguments passed. Please see the corresponding method's
 * documentation for the details of each contract.
 *
 * @tparam T Payload data type. Required to be default-constructible.
 */
template<typename T>
class PayloadType
{
public:
	/** @brief A default constructor. */
	PayloadType() = default;
	/** @brief An empty virtual destructor. */
	virtual ~PayloadType() = default;

	/** @brief A default copy constructor. */
	PayloadType(const PayloadType&) = default;
	/** @brief A default move constructor. */
	PayloadType(PayloadType&&) = default;
	/** @brief A default copy-assignment operator. */
	PayloadType& operator=(const PayloadType&) = default;
	/** @brief A default move-assignment operator. */
	PayloadType& operator=(PayloadType&&) = default;

	/**
	 * @brief Data serialization method.
	 *
	 * Plays a dual role in the serialization process:
	 * 1. it writes `val` as a bytestream to the output buffer _if possible_;
	 * 2. it informs of the length of the bytestream via the return value.
	 *
	 * These two roles are largely independent of each other. We will elaborate
	 * on them in the next two paragraphs.
	 *
	 * For the first role: the function might or might not serialize the value
	 * `val` depending on the output buffer capacity `bufferSize`. Throughout the
	 * implementation it is assumed that if the return value `payloadSize` is
	 * strictly larger than `bufferSize`, then the buffer content is invalid and
	 * is to be disregarded. Otherwise, the buffer content is treated as a valid
	 * payload to be stored in the database and is sent over the channel to a
	 * connected TStorage instance. The deserialized payload is never reused
	 * after a send (successful or not).
	 *
	 * For the second role: the implementation requires that an invocation of
	 * this method with a valid argument `val` returns the expected bytestream
	 * length of `val` whether or not the serialization is actually performed due
	 * to insufficient output buffer capacity (see the paragraph above). In
	 * particular, a call to this method with `bufferSize=0` should act as a
	 * function returning the serialized bytestream length of `val` without
	 * perfoming the serialization itself. In this case the implementation should
	 * allow `outputBuffer` to be a `nullptr`.
	 *
	 * The client operates on an additional extensionality assumption and expects
	 * it to be satisfied: any two calls to this method with the same argument
	 * `val` (meaning not just equal, as in `val1 == val2`, but *identical*, as
	 * in `&val1 == &val2`) should produce the same bytestream as a result, given
	 * a large enough `outputBuffer`.
	 *
	 * Care must be taken for data which is difficult or expensive to serialize.
	 * The channel doesn't know the required size of the buffer in advance, hence
	 * it might supply the user with a buffer of insufficient size, resulting in
	 * a failed first attempt at serialization and a retry with a known requested
	 * buffer size (as returned by the first call), potentially doubling the
	 * serialization cost in some edge cases if implemented naively. Some caching
	 * mechanism, guided by the extensionality assumption above and the
	 * no-payload-reuse guarantee, might prove helpful in mitigating that.
	 *
	 * @param[in] val Payload to serialize. Each value of type T is treated as a
	 * valid payload and is expected to undergo serialization without errors
	 * given a large enough buffer.
	 *
	 * @param[out] outputBuffer Address of the output buffer (a writable block of
	 * memory) of size `bufferSize`.
	 *
	 * @param[in] bufferSize Length of the output buffer.
	 *
	 * @return Payload size.
	 */
	virtual std::size_t toBytes(
		const T& val, void* outputBuffer, std::size_t bufferSize) = 0;

	/**
	 * @brief Data deserialization method.
	 *
	 * Expected to deserialize data contained in `payloadBuffer` if possible.
	 * On success, it should store the result in `oVar` and return `true`, or
	 * return `false` on deserialization failure.
	 *
	 * The initial value of `oVar` is guaranteed to be a default-constructed
	 * instance of type `T`.
	 *
	 * @param[out] oVar A reference to a variable of type T where the result of
	 * deserialization will be stored.
	 *
	 * @param[in] payloadBuffer A buffer (contiguous block of memory of length at
	 * least `payloadSize`) containing the payload in a bytestream form.
	 *
	 * @param[in] payloadSize Length of the input buffer.
	 *
	 * @return `true` if deserialization was successful, `false` otherwise.
	 */
	virtual bool fromBytes(
		T& oVar, const void* payloadBuffer, std::size_t payloadSize) = 0;
};

} /*namespace tstorage*/

#endif
