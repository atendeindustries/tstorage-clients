/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_LOG_H
#define D_TSTORAGE_EXAMPLECSV_LOG_H

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace tstorage {
namespace exampleCSV {

class Log
{
public:
	class Stream
	{
	public:
		Stream(std::ostream& stream) : mStream(&stream) {}
		~Stream();

		Stream(const Stream&) = delete;
		Stream(Stream&& other) noexcept;
		Stream& operator=(const Stream&) = delete;
		Stream& operator=(Stream&&) noexcept;

		template<typename T>
		Stream& operator<<(T&& x) &;

		template<typename T>
		Stream&& operator<<(T&& x) &&;

	private:
		std::ostream* mStream;
	};

	Log(std::string header = "")
		: mOut(&std::cerr), mHeader(std::move(header)), mIndent(false)
	{
	}
	Log(std::ostream& ostr, std::string header = "")
		: mOut(&ostr), mHeader(std::move(header)), mIndent(false)
	{
	}

	template<typename T>
	Stream operator<<(T&& x)
	{
		if (!mHeader.empty()) {
			*mOut << mHeader << "\n";
			mHeader = "";
			mIndent = true;
		}
		if (mIndent) {
			*mOut << "  ";
		}
		*mOut << static_cast<std::decay_t<T>>(std::forward<T>(x));
		return Stream(*mOut);
	}

	Stream error() { return *this << "error: "; }

private:
	std::ostream* mOut;
	std::string mHeader;
	bool mIndent;
};

template<typename T>
Log::Stream& Log::Stream::operator<<(T&& x) &
{
	if (mStream != nullptr) {
		*mStream << static_cast<std::decay_t<T>>(std::forward<T>(x));
	}
	return *this;
}

template<typename T>
Log::Stream&& Log::Stream::operator<<(T&& x) &&
{
	if (mStream != nullptr) {
		*mStream << static_cast<std::decay_t<T>>(std::forward<T>(x));
	}
	return std::move(*this);
}

/* An explicit instantiation for const char*. Without it we would have as many
 * implicit instantiations as the number of unique sizes of string literals
 * used as the RHS of the operator (which is many, due to a copious amount of
 * log messages sprinkled throughout the code). */

template Log::Stream Log::operator<< <const char*>(const char*&&);

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
