/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "../File.h"
#include "TmpDir.h"

#include <catch2/catch_test_macros.hpp>

using namespace tstorage::exampleCSV;

TEST_CASE("File", "[file]")
{
	TmpDir tmpDir;
	REQUIRE(tmpDir.valid());

	std::string filename = tmpDir.pathToFile("things.txt");
	std::fstream fstr(filename, std::fstream::out);
	std::vector<std::string> lines{
		"hello",
		"HELLO!",
		"1234312",
		"!@#$%^&*()",
		"\\",
		"/",
		"   ",
		"   32 a ",
		"some spaces   ",
		"     before and after   ",
		"",
		"",
		"\t\t",
		"",
		"",
		"???",
		"",
		"",
		"",
		"",
		"foo",
		"",
		"+_+_+_",
		"",
		"123, 213, 4123, 324, 123123, 0.214214",
	};
	for (const auto& line : lines) {
		fstr << line << "\n";
	}
	fstr.flush();
	fstr.close();

	File file;
	SECTION("two-step init")
	{
		File f;
		f.open(filename);
		REQUIRE(!f.error());
		file = std::move(f);
	}
	SECTION("one-step init")
	{
		File f{filename};
		REQUIRE(!f.error());
		file = std::move(f);
	}
	REQUIRE(!file.error());
	REQUIRE(file.name() == filename);

	std::size_t i = 0;
	while (file) {
		REQUIRE(file.readLine() == lines[i]);
		++i;
	}
	REQUIRE(!file.error());
	REQUIRE(file.eof());
	file.close();
	REQUIRE(std::remove(filename.c_str()) == 0);
	REQUIRE(tmpDir.remove());
}
