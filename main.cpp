#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <stack>
#include <unordered_set>

#include "esmel_compiler.h"
#include "esmel_interpreter.h"

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::nullptr_t, std::shared_ptr,
		std::unordered_set;


int main(int argc, char* argv[])
{
	if (argc == 2) {
		std::cout << std::format(	""
	"      *          Welcome to the Esmel Language!\n"
	"     ***         Author: Sharll\n"
	"   *******       Version: v3.7-official-pre-release-1\n"
	"*************    Usage:\n"
	"   *******           {} your_esmel_code.esm\n"
	"     ***         \n"
	"      *          Hope you'll have a pleasant journey!", argv[0]) << std::endl;
		return 0;
	}

	// string file =  argv[1];
	string file = R"(C:\Users\Sharll\CLionProjects\Esmel_Lang\test.esm)";
	string mainfunc = "main";

	esmel_compiler e{};
	e.add_target(file);
	e.compile();

	EsmelInterpreter esm;
	esm.functions = e.esmel_functions;
	esm.call(0);

	// for (auto i: e.esmel_functions) {
	// 	cout << "Function(" << i.arguments << ')' << endl;
	//
	// 	std::cout << "\tstatic_strs: ";
	//
	// 	for (auto j: i.static_strs) std::cout << j << ' ';
	//
	// 	std::cout << std::endl << "\tbyte_code: " << endl;
	//
	// 	for (auto j : i.code) {
	// 		for (auto k: j) {
	// 			printf("\t\t%d,%lld", k.op, k.data);
	// 		}
	// 		std::cout << std::endl;
	// 	}
	//
	// }
	return 0;

	for (auto i: e.preloaded_codes) {
		std::cout << "function " << i.first << ":" << std::endl << "\targuments: " << i.second.arguments;
		std::cout << std::endl << "\tcode: ";
		for (auto j = 0; j < i.second.code.size(); j++) {
			cout << i.second.real_line_num[j] << '\t';
			for (auto k = 0; k<i.second.code[j].size(); k++) {
				cout << i.second.code[j][k] << ' ';
			}
			std::cout << std::endl << '\t';
		}
		cout << "flags: " << endl;
		for (auto j : i.second.temp_flags_record) {
			cout << j.first << ' ' << j.second << endl;
		}
		cout << "variables: " << endl;
		for (auto j : i.second.temp_variable_record) {
			cout << j.first << ' ' << j.second << endl;
		}

	}


	return 0;
}
