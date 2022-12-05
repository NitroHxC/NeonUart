#include <iostream>
#include <stdint.h>

extern void example1();

void print_debug(uint8_t* buf, uint16_t len)
{
	// Print outbound buffer
	std::cout << "Message: " << std::endl;
	for (uint8_t i = 0; i < len; i++)
	{
		std::cout << "0x" << std::hex << (int)buf[i];
		if (i != len - 1) std::cout << ",";
	}
	std::cout << std::endl;
}

int main()
{
	std::cout << "Hello Neon!\n";

	example1();
}
