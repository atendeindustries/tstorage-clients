/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "File.h"

#include <locale>
#include <string>

namespace tstorage {
namespace exampleCSV {

void File::open(const std::string& path)
{
	mFile.open(path);
	mFileName = path;
	if (mFile) {
		mFile.imbue(std::locale("C"));
	}
	mFile.peek();
}

void File::close()
{
	mFile.close();
	mFileName = "";
}

std::string File::readLine()
{
	if (!lineAvailable()) {
		return "";
	}
	std::string str{};
	std::getline(mFile, str);
	if (error()) { /* so that the subsequent peek doesn't reset ERRNO */
		return str;
	}
	mFile.peek(); /* induces EOF before the next readLine if no chars are available */
	++mLineNumber;
	return str;
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
