#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <memory>
#include <stack>
#include <unordered_set>

#include "EsmelInterpreter.h"

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::nullptr_t, std::shared_ptr,
		std::unordered_set;


int main(int argc, char* argv[])
{
	if (argc != 2) return -1;

	string file =  argv[1];
	string mainfunc = "main";

	EsmelInterpreter e;

	e.run(file, mainfunc);

	return 0;
}
