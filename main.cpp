#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <stack>
#include <unordered_set>

#include "esmel_bytecode.h"
#include "esmel_compiler.h"
#include "esmel_interpreter.h"

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::nullptr_t, std::shared_ptr,
		std::unordered_set;


int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << 	""
	"      *          Welcome to the Esmel Language!\n"
	"     ***         Author: Sharll\n"
	"   *******       Version: v3.7-official-pre-release-4\n"
	"*************    To run a program directly, use `esmel your_esmel_code.esm`\n"
	"   *******       To compile a program,      use `esmel compile your_esmel_code.esm`\n"
	"     ***         Hope you'll have a pleasant journey!\n"
	"      *          To get further informationn, visit https://github.com/Sharll-large/Esmel" << std::endl;
		return 0;
	}
	if (argc == 2) {
		// string file =  argv[1];
		string file = argv[1];
		string mainfunc = "main";

		auto* e = new esmel_compiler();
		e->add_target(file);
		e->compile();

		EsmelInterpreter esm;
		esm.functions = e->esmel_functions;
		esm.static_str = e->static_strs;

		delete e;

		esm.call(0);
	}
	if (argc == 3) {
		if (std::string(argv[1]) == "compile") {
			cout << "Compiling " << argv[2] << "..." << endl;
			string filename = argv[2];				// xxx.esm
			string target = filename + "el";	// xxx.esmel

			std::ofstream f(target, std::ios::binary | std::ios::out);

			auto e = esmel_compiler();
			e.add_target(filename);
			e.compile();

			EsmelByteCode::write(&e.esmel_functions, &f);

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
		}
	}
	return 0;
	//



	return 0;
}
