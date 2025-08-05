/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

#include "../CsvRecord.h"

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace tstorage::exampleCSV;

TEST_CASE("CsvRecord: split", "[record]")
{
	std::tuple<std::string, std::vector<std::string>> data =
		GENERATE(table<std::string, std::vector<std::string>>({
			{"Long piece of text", {"Long piece of text"}},
			{"cid, mid, moid", {"cid", " mid", " moid"}},
			{"a,b,c,d,e,f,g,h,i,j", {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"}},
			{"a,b,c,d,e,f,gh,ij", {"a", "b", "c", "d", "e", "f", "gh", "ij"}},
			{"a,b,c,d,e,fgh,ij", {"a", "b", "c", "d", "e", "fgh", "ij"}},
			{"a,b,c,d,efgh,ij", {"a", "b", "c", "d", "efgh", "ij"}},
			{"", {}},
			{",", {"", ""}},
			{",,,,", {"", "", "", "", ""}},
			{",, ,,", {"", "", " ", "", ""}},
			{"-2147483648,0,0,0,1.13505e-43",
				{"-2147483648", "0", "0", "0", "1.13505e-43"}},
		}));
	const std::string& text = std::get<0>(data);
	const std::vector<std::string>& fields = std::get<1>(data);
	const int nFields = static_cast<int>(fields.size());

	CsvRecord record(text);
	CHECK(record.countFields() == nFields);
	INFO("line  " << text);
	for (int i = 0; i < std::min(CsvRecord::cNumberOfFields, record.countFields()); ++i) {
		std::string field = record.getField(i);
		INFO("field " << i);
		REQUIRE(field == fields[i]);
	}
}
