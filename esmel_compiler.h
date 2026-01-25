#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "esmel_callable.h"
#include <iostream>
#include <fstream>

#define main_func_name "Main"

using namespace std;


class esmel_compiler {
	struct preloaded_code {
		size_t id{};
		string name;
		string file_name;
		size_t arguments{};
		std::vector<uint64_t> real_line_num;
		unordered_map<string, uint64_t> temp_variable_record;
		unordered_map<string, uint64_t> temp_flags_record;
		std::vector<std::vector<std::string>> code;
		std::unordered_set<std::string> keywords;
	};
public:
	vector<esmel_function> esmel_functions;
	unordered_map<string, preloaded_code> preloaded_codes;
	std::unordered_map<std::string, uint64_t> static_strs_record;
	std::vector<std::string> static_strs;

	esmel_compiler() {
		preloaded_codes = {
			{main_func_name, {}}
		};
	}

	static vector<string> splitLine(const string &line, int line_num)
	{
		vector<string> tokens;
		string current_token;
		bool in_quotes = false;
		int quote_start_pos = -1;

		for (size_t i = 0; i < line.length(); ++i)
		{
			char c = line[i];

			if (c == '"')
			{
				if (!in_quotes)
				{
					// 开始引号
					quote_start_pos = i;
				}
				else
				{
					// 结束引号
					quote_start_pos = -1;
				}
				in_quotes = !in_quotes;
				current_token += c;
			}
			else if (std::isspace(c) && !in_quotes)
			{
				if (!current_token.empty())
				{
					tokens.push_back(current_token);
					current_token.clear();
				}
			}
			else if (c == '#' && !in_quotes) {
				break;
			}
			else
			{
				current_token += c;
			}
		}

		// 检查引号是否匹配
		if (in_quotes)
		{
			std::cerr << "Unclosed quotes at line " << line_num + 1 << ", position " << quote_start_pos + 1;
			exit(-1);
		}

		if (!current_token.empty())
		{
			tokens.push_back(current_token);
		}

		return tokens;
	}

	static vector<vector<string>> splitLinesFromFile(const string &filename)
	{
		vector<vector<string> > result;
		std::ifstream file(filename);

		if (!file.is_open())
		{
			std::cerr << "Error: Cannot open file: " << filename << std::endl;
			exit(0);
		}

		string line;
		int line_num = 0;

		while (std::getline(file, line))
		{
			result.push_back(splitLine(line, line_num));
			line_num++;
		}

		file.close();
		return result;
	}


	void add_target(string &filename) {
		// 将一个目标Esmel源代码文件加入预编译。
		auto parsed = splitLinesFromFile(filename);
		// 目前的函数名
		string current = main_func_name;
		for (uint64_t i = 0; i < parsed.size(); i++) {
			if (parsed[i].empty()) continue;
			if (parsed[i][0] == "Function") {
				if (parsed[i].size() < 2) {
					std::cerr << "Error: Empty function defined.\n\tat " << filename << ':' << i+1 << std::endl;
					exit(0);
				}
				if (!std::isupper(parsed[i][1][0])) {
					std::cerr << "Error: Function name must be started a uppercase letter. (Consider using \'" << static_cast<char>(std::toupper(parsed[i][1][0]))
							<< parsed[i][1].substr(1) << "\')\n\tat " << filename << ':' << i+1 << std::endl;
					exit(0);
				}
				auto t = preloaded_codes.find(parsed[i][1]);
				if (t == preloaded_codes.end()) {
					// 新定义函数则分配一个id
					preloaded_codes.insert({parsed[i][1], {preloaded_codes.size()}});
				} else {
					preloaded_codes[parsed[i][1]].temp_variable_record.clear();
					preloaded_codes[parsed[i][1]].code.clear();
					preloaded_codes[parsed[i][1]].temp_flags_record.clear();
					preloaded_codes[parsed[i][1]].real_line_num.clear();
				}
				current = parsed[i][1];
				preloaded_codes[current].name = current;
				preloaded_codes[current].file_name = filename;
				preloaded_codes[current].arguments = parsed[i].size() - 2;
				preloaded_codes[current].keywords.insert(current);
				for (size_t j=2; j<parsed[i].size(); j++) {
					if (preloaded_codes[current].keywords.contains(parsed[i][j])) {
						std::cerr << "Error: Redefined keyword \'" << parsed[i][j] << "\'\n\tat file " << filename << ':' << i+1 << std::endl;
						exit(0);
					}
					preloaded_codes[current].temp_variable_record[parsed[i][j]] = preloaded_codes[current].temp_variable_record.size();
					preloaded_codes[current].keywords.insert(parsed[i][j]);
				}
			} else if (parsed[i][0] == "Flag") {
				if (parsed[i].size() != 2) {
					std::cerr << "Error: Illegal flag defined.\n\tat file " << filename << ':' << i+1 << std::endl;
					exit(0);
				}
				if (preloaded_codes[current].keywords.contains(parsed[i][1])) {
					std::cerr << "Error: Redefined keyword \'" << parsed[i][1] << "\'\n\tat " << filename << ':' << i+1 << std::endl;
					exit(0);
				}
				if (!std::isupper(parsed[i][1][0])) {
					std::cerr << "Error: Flag name must be started a uppercase letter. (Consider using \'" << static_cast<char>(std::toupper(parsed[i][1][0]))
						<< parsed[i][1].substr(1) << "\')\n\tat " << filename << ':' << i+1 << std::endl;
					exit(0);
				}
				preloaded_codes[current].temp_flags_record[parsed[i][1]] = preloaded_codes[current].code.size();
				preloaded_codes[current].keywords.insert(parsed[i][1]);
			} else {
				preloaded_codes[current].code.push_back(parsed[i]);
				preloaded_codes[current].real_line_num.push_back(i+1);
			}
		}
	}

	void compile()
	{
		// 编译
		esmel_functions = vector<esmel_function>(preloaded_codes.size());
		for (const auto& i: preloaded_codes) {
			unordered_map<string, uint64_t> temp_variable_record = i.second.temp_variable_record;
			unordered_map<string, uint64_t> temp_flags_record = i.second.temp_flags_record;
			esmel_function current_func = esmel_function();
			current_func.real_line_num = i.second.real_line_num;
			current_func.arguments = i.second.arguments;
			current_func.name = i.second.name;
			current_func.file_name = i.second.file_name;
			current_func.variable_count = i.second.arguments;
			for (size_t j = 0; j < i.second.code.size(); j++) {
				current_func.code.emplace_back();
				for (auto it = i.second.code[j].rbegin(); it != i.second.code[j].rend(); ++it) {
					string token = *it;
					if (token.length() >= 2 && token[0] == '\"' && token[token.size()-1] == '\"') {
						// 字符串。
						token = token.substr(1, token.length() - 2);
						if (static_strs_record.find(token) == static_strs_record.end()) {
							// 添加字符串字面量。
							static_strs_record[token] = static_strs_record.size();
						}
						current_func.code.back().push_back({operation::GetStaticStr, static_strs_record[token]});
					}
					// // 结构符（应该总是在行初遇到）
					// else if (token == "While") {
					// 	current_func.code.back().push_back({operation::Goto, -1});
					// }
					// else if (token == "End") {
					//
					// }

					// 布尔值。
					else if (token == "True") current_func.code.back().emplace_back(operation::CreateBoolean, true);
					else if (token == "False") current_func.code.back().emplace_back(operation::CreateBoolean, false);
					else if (token == "Undefined") current_func.code.back().emplace_back(operation::CreateUndefined, 0);
					else if (builtin.contains(token)) {
						current_func.code.back().push_back({builtin.at(token), 0});
					} else if (vari_only_builtin.contains(token)) {
						// 特殊：Set操作
						if (current_func.code.back().back().op != operation::GetVar) {
							cerr << "Illegal " << token << ". This method can only be used on variables.\n\tat " << i.second.file_name << ':' << i.second.real_line_num[j];
							exit(-1);
						}
						current_func.code.back().back() = {vari_only_builtin.at(token), current_func.code.back().back().data};
					} else if (types.contains(token)){
						current_func.code.back().emplace_back(operation::CreateType, std::bit_cast<int32_t>(types.at(token)));
					}else {
						// 尝试解析为整数
						long long llvalue;
						auto [ptr, ec] = std::from_chars(token.data(), token.data()+token.size(), llvalue);
						if (ec == std::errc() && ptr == token.data() + token.size()) {
							current_func.code.back().push_back({operation::CreateInt, std::bit_cast<uint64_t>(llvalue)});
						} else {
							// 浮点数
							double dbvalue;
							auto [ptr2, ec2] = std::from_chars(token.data(), token.data()+token.size(), dbvalue);
							if (ec2 == std::errc() && ptr2 == token.data() + token.size()) {
								// std::cout << "good " << dbvalue;
								current_func.code.back().push_back({operation::CreateFloat, std::bit_cast<uint64_t>(dbvalue)});
							}
							// 运行时变量 或 flag
							else if (temp_flags_record.find(token) != temp_flags_record.end()) {
								// 如果这是一个flag。
								current_func.code.back().push_back({operation::Goto, temp_flags_record.at(token)});
							} else {
								if (std::isupper(token[0])) {
									// 开头大写，作为函数解析
									if (preloaded_codes.find(token) == preloaded_codes.end()) {
										// 未找到函数则报错
										cerr << "Cannot find function or flag \'" << token << "\'. If you means a variable, consider using a lowercase letter started word." << "(Like \'"
											<< static_cast<char>(std::tolower(token[0])) << token.substr(1) << "\')\n\tat " << i.second.file_name << ':' << i.second.real_line_num[j];
										exit(-1);
									}
									// 如果是Esmel函数
									current_func.code.back().push_back({operation::Call, preloaded_codes[token].id});
								} else {
									// 否则判定为运行时变量。
									if (invalid.contains(token)) {
										cerr << "\'" << token << "\' is an invalid variable name. Perhaps you mean " << invalid.at(token) << std::endl;
										cerr << "\tat " << i.second.file_name << ':' << i.second.real_line_num[j];
									} else {
										if (temp_variable_record.find(token) == temp_variable_record.end()) {
											// 第一次遇见此变量，则为此变量分配一个ID。
											temp_variable_record[token] = temp_variable_record.size();
											current_func.variable_count += 1;
										}
										current_func.code.back().push_back({operation::GetVar, temp_variable_record[token]});
									}
								}
							}
						}
					}
				}
			}
			esmel_functions[i.second.id] = current_func;
		}
		static_strs.resize(static_strs_record.size());
		for (const auto& [i, j] : static_strs_record) {
			static_strs[j] = i;
		}
	}
};