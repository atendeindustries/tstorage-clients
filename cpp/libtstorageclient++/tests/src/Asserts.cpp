#include "Asserts.h"

#include <iomanip>
#include <ios>

namespace tstorage {

void print_mem(const void *x, std::size_t size)
{
	std::ios_base::fmtflags flags(cout.flags());
	for (std::size_t i = 0; i < size; ++i) {
		unsigned byte = *(static_cast<const unsigned char*>(x) + i);
		cout << std::hex << std::setw(2) << std::setfill('0') << byte;
		if (i != size) {
			cout << " ";
		}
	}
	cout.flags(flags);
}

} /*namespace tstorage*/
