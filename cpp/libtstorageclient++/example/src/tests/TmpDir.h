/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_TESTS_TMPDIR_H
#define D_TSTORAGE_EXAMPLECSV_TESTS_TMPDIR_H

#include <stdlib.h>
#include <unistd.h>
#include <string>

namespace tstorage {
namespace exampleCSV {

class TmpDir
{
	static constexpr const char* const cDirTemplate = "/tmp/tscsvtest.XXXXXX";

public:
	TmpDir() : mPath(cDirTemplate)
	{
		char* tmpdir = mkdtemp(static_cast<char*>(&mPath.front()));
		if (tmpdir == nullptr) {
			mPath = "";
		}
	}

	~TmpDir() { remove(); }

	TmpDir(const TmpDir&) = delete;
	TmpDir(TmpDir&&) = default;
	TmpDir& operator=(const TmpDir&) = delete;
	TmpDir& operator=(TmpDir&&) = default;

	bool valid() const { return !mPath.empty(); }
	std::string pathToFile(const std::string& filename) const
	{
		return mPath + "/" + filename;
	}
	std::string pathToDir() const { return mPath; }
	bool remove()
	{
		if (valid()) {
			bool success = rmdir(mPath.c_str()) == 0;
			mPath = "";
			return success;
		}
		return true;
	}

private:
	std::string mPath;
};

} /*namespace exampleCSV*/
} /*namespace tstorage*/

#endif
