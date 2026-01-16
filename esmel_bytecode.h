#pragma once
#include <fstream>
#include <vector>

#include "esmel_callable.h"

constexpr uint16_t ver = 0x0374;
const char MAGIC[] = "esmel_bytecode";

class EsmelByteCode {
public:
	static void write(std::vector<esmel_function>* source, std::ofstream* target) {
		target->write(MAGIC, sizeof(MAGIC) - 1);
		target->write(reinterpret_cast<const char*>(&ver), sizeof(ver));

		uint64_t functionCount = static_cast<uint32_t>(source->size());
		target->write(reinterpret_cast<const char*>(&functionCount),
								 sizeof(functionCount));
	}
	static void read(std::ofstream* source, std::vector<esmel_function>* target) {

	}
};
