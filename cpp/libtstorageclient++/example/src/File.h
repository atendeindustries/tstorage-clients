/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_FILE_H
#define D_TSTORAGE_EXAMPLECSV_FILE_H

#include <fstream>
#include <string>

namespace tstorage {
namespace exampleCSV {

class File
{
public:
	File() = default;
	explicit File(std::string name) : File() { open(name); }
	~File() { close(); }

	File(const File&) = delete;
	File(File&&) noexcept = default;
	File& operator=(const File&) = delete;
	File& operator=(File&&) noexcept = default;

	void open(const std::string& path);
	void close();

	std::string readLine();

	std::string name() const { return mFileName; }
	int lineNumber() const { return mLineNumber; }
	bool lineAvailable() const { return mFile.good(); }
	operator bool() const { return lineAvailable(); }

	bool error() const { return mFile.fail(); }
	bool eof() const { return !lineAvailable() && !error(); }
	bool isOpen() const { return mFile.rdbuf()->is_open(); }

private:
	std::ifstream mFile;
	std::string mFileName;
	int mLineNumber{};
};

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
