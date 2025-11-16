//
// Created by Sharll on 2025/11/2.
//

#pragma once

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <stack>
#include <memory>
#include <sstream>
#include <unordered_set>

#include "esmel_function.h"
#include "QuoteParseException.h"

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::shared_ptr,
		std::unordered_set;

class EsmelInterpreter
{
	vector<vector<string> > codes;
	vector<int> real_line_num;
	unordered_map<string, esmel_function> esmel_functions;

	// 存储变量。
	vector<int> int_store_;
	vector<double> double_store_;
	vector<bool> bool_store_;
	vector<string> string_store_;
	unordered_set<string> keyword_store_; // 存储flags和变量名
	vector<vector<EsmelObject> > array_store_;
	unordered_map<Type, stack<size_t> > free_indices_;

	string to_string(EsmelObject o)
	{
		switch (o.type)
		{
		case Type::INT: return std::to_string(int_store_[o.index]);
		case Type::DOUBLE: return std::to_string(double_store_[o.index]);
		case Type::BOOL: return bool_store_[o.index] ? "true" : "false";
		case Type::STRING: return string_store_[o.index];
		case Type::ARRAY:
		{
			const auto &arr = array_store_[o.index];
			string result = "[";
			for (size_t i = 0; i < arr.size(); ++i)
			{
				result += to_string(arr[i]);
				if (i < arr.size() - 1)
				{
					result += ", ";
				}
			}
			result += "]";
			return result;
		}
		case Type::NULL_T:
			return "null";
		}
	}

	string type_of(EsmelObject target)
	{
		switch (target.type)
		{
		case Type::INT: return "int";
		case Type::DOUBLE: return "float";
		case Type::BOOL: return "bool";
		case Type::STRING: return "string";
		case Type::ARRAY: return "array";
		case Type::NULL_T: return "null";
		}
	}

	bool is_(EsmelObject o1, EsmelObject o2)
	{
		return o1.type == o2.type && o1.index == o2.index;
	}

	bool equal_(EsmelObject o1, EsmelObject o2)
	{
		if (o1.type != o2.type) return false;
		switch (o1.type)
		{
		case Type::INT: return int_store_[o1.index] == int_store_[o2.index];
		case Type::DOUBLE: return double_store_[o1.index] == double_store_[o2.index];
		case Type::BOOL: return bool_store_[o1.index] == bool_store_[o2.index];
		case Type::STRING: return string_store_[o1.index] == string_store_[o2.index];
		case Type::ARRAY:
		{
			auto a1 = array_store_[o1.index];
			auto a2 = array_store_[o2.index];
			if (a1.size() != a2.size()) return false;
			for (int i = 0; i < a1.size(); i++)
			{
				if (!equal_(a1[i], a2[i])) return false;
			}
			return true;
		}
		case Type::NULL_T: return true;
		}
	}

	EsmelObject createInt(int value)
	{
		size_t index;
		if (!free_indices_[Type::INT].empty())
		{
			index = free_indices_[Type::INT].top();
			free_indices_[Type::INT].pop();
			int_store_[index] = value;
		}
		else
		{
			index = int_store_.size();
			int_store_.push_back(value);
		}
		return {Type::INT, index};
	}

	EsmelObject createDouble(double value)
	{
		size_t index;
		if (!free_indices_[Type::DOUBLE].empty())
		{
			index = free_indices_[Type::DOUBLE].top();
			free_indices_[Type::DOUBLE].pop();
			double_store_[index] = value;
		}
		else
		{
			index = double_store_.size();
			double_store_.push_back(value);
		}
		return {Type::DOUBLE, index};
	}

	EsmelObject createBool(bool value)
	{
		size_t index;
		if (!free_indices_[Type::BOOL].empty())
		{
			index = free_indices_[Type::BOOL].top();
			free_indices_[Type::BOOL].pop();
			bool_store_[index] = value;
		}
		else
		{
			index = bool_store_.size();
			bool_store_.push_back(value);
		}
		return {Type::BOOL, index};
	}

	EsmelObject createString(const string &value)
	{
		size_t index;
		if (!free_indices_[Type::STRING].empty())
		{
			index = free_indices_[Type::STRING].top();
			free_indices_[Type::STRING].pop();
			string_store_[index] = value;
		}
		else
		{
			index = string_store_.size();
			string_store_.push_back(value);
		}
		return {Type::STRING, index};
	}

	EsmelObject createArray(const vector<EsmelObject> &value = {})
	{
		size_t index;
		if (!free_indices_[Type::ARRAY].empty())
		{
			index = free_indices_[Type::ARRAY].top();
			free_indices_[Type::ARRAY].pop();
			array_store_[index] = value;
		}
		else
		{
			index = array_store_.size();
			array_store_.push_back(value);
		}
		return {Type::ARRAY, index};
	}

	EsmelObject createNull()
	{
		return {Type::NULL_T, 0}; // index对于null类型不重要
	}

	// 获取值的方法
	int &getInt(const EsmelObject &obj)
	{
		assert(obj.type == Type::INT);
		return int_store_[obj.index];
	}

	double &getDouble(const EsmelObject &obj)
	{
		assert(obj.type == Type::DOUBLE);
		return double_store_[obj.index];
	}

	bool getBool(const EsmelObject &obj)
	{
		assert(obj.type == Type::BOOL);
		return bool_store_[obj.index];
	}

	string &getString(const EsmelObject &obj)
	{
		assert(obj.type == Type::STRING);
		return string_store_[obj.index];
	}

	vector<EsmelObject> &getArray(const EsmelObject &obj)
	{
		assert(obj.type == Type::ARRAY);
		return array_store_[obj.index];
	}

	EsmelObject parse_object(string &origin)
	// 将字符串转为一个Esmel对象。
	{
		// 已定义的变量
		auto &var = stack_frame.top().variables;
		if (var.find(origin) != var.end()) return stack_frame.top().variables[origin];
		// 字符串
		if (origin.length() >= 2 && origin[0] == '\"' && origin[origin.length() - 1] == '\"')
		{
			// 去除引号，返回字符串内容
			string content = origin.substr(1, origin.length() - 2);
			return createString(content);
		}
		// 布尔值
		if (origin == "true") return createBool(true);
		if (origin == "false") return createBool(false);
		// 整数
		try
		{
			// 检查是否全是数字（可能带负号）
			if (!origin.empty() && (isdigit(origin[0]) || (origin[0] == '-' && origin.length() > 1 && isdigit(origin[1]))))
			{
				// 尝试转换为整数
				size_t pos;
				int int_value = std::stoi(origin, &pos);
				// 如果整个字符串都被成功转换
				if (pos == origin.length())
				{
					return createInt(int_value);
				}
			}
		}
		catch (const std::exception &_)
		{
		}
		// 浮点数或科学计数法
		try
		{
			// 检查是否包含小数点或者是科学计数法
			if (origin.find('.') != string::npos || origin.find('e') != string::npos || origin.find('E') != string::npos)
			{
				size_t pos;
				double double_value = std::stod(origin, &pos);
				if (pos == origin.length())
				{
					return createDouble(double_value);
				}
			}
		}
		catch (const std::exception &_)
		{
		}

		if (origin == "[]" || origin == "array") return createArray();

		std::cerr << "Error: can not parse the element \"" << origin << "\"" << std::endl;
		error();
	}

	unordered_map<string, EsmelObject> parse_cache; // 解析缓存（暂未实现）

	struct frame // 栈帧
	{
		unordered_map<string, EsmelObject> variables;
		string &funcname;
		int on_line;
	};

	stack<frame> stack_frame; // 栈帧（顶部表示当前的栈帧，存储局部变量信息。）
	unordered_map<string, int> flags; // 标记
	stack<EsmelObject> exec_stack; // 执行栈
	unordered_set<string> keywords;

	vector<string> splitLine(const string &line, int line_num)
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
			else
			{
				current_token += c;
			}
		}

		// 检查引号是否匹配
		if (in_quotes)
		{
			throw QuoteParseException("Unclosed quotes", line_num + 1, quote_start_pos + 1);
		}

		if (!current_token.empty())
		{
			tokens.push_back(current_token);
		}

		return tokens;
	}

	vector<vector<string> > splitLines(const string &text)
	{
		vector<vector<string> > result;
		std::istringstream iss(text);
		string line;
		int line_num = 0;

		while (std::getline(iss, line))
		{
			try
			{
				result.push_back(splitLine(line, line_num));
			}
			catch (const QuoteParseException &e)
			{
				std::cerr << "Error: " << e.what() << " at line " << e.getLineNumber()
						<< ", position " << e.getPosition() << std::endl;
				exit(EXIT_FAILURE);
			}
			line_num++;
		}

		return result;
	}

	vector<vector<string> > splitLinesFromFile(const string &filename)
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
			try
			{
				result.push_back(splitLine(line, line_num));
			}
			catch (const QuoteParseException &e)
			{
				std::cerr << "Error: " << e.what() << " at line " << e.getLineNumber()
						<< ", position " << e.getPosition() << std::endl;
				exit(EXIT_FAILURE);
			}
			line_num++;
		}

		file.close();
		return result;
	}

	void call(string &func_name)
	// 调用一个非内置的esmel函数。
	{
		auto func = esmel_functions[func_name];
		unordered_map<string, EsmelObject> args;

		for (int i = 0; i < func.arguments.size(); i++)
		{
			if (exec_stack.empty())
			{
				std::cerr << "Error: " << func_name << " needs " << func.arguments.size() << " argument(s), but only " << i <<
						" are given." << std::endl;
				error();
			}
			args[func.arguments[i]] = exec_stack.top();
			exec_stack.pop();
		}

		stack_frame.push({args, func_name, func.start});

		for (int i = func.start; i < func.end;)
		{
			stack_frame.top().on_line = i = exec_line(codes[i], i);
		}
	}

	void error()
	// 打印调用栈并非正常退出。
	{
		while (!stack_frame.empty())
		{
			auto st = stack_frame.top();
			std::cerr << "\tat\t" << st.funcname << '(' << real_line_num[st.on_line] << ")" << std::endl;
			stack_frame.pop();
		}
		exit(EXIT_FAILURE);
	}

	int exec_line(vector<string> &code, int line)
	// 执行一段esmel代码。
	{
		for (int i = code.size() - 1; i >= 0; i--)
		{
			auto token = code[i];
			// 控制流
			if (token == "goto")
			{
				string target = getString(exec_stack.top());
				exec_stack.pop();
				if (flags.find(target) != flags.end())
				{
					return flags[target];
				}
				std::cerr << "Error: can not find flag \"" << target << "\"" << std::endl;
				error();
			}
			else if (token == "if")
			{
				auto val = getBool(exec_stack.top());
				exec_stack.pop();
				if (!val) return line + 1;
			}
			else if (token == "return")
			{
				stack_frame.pop();
				return code.size();
			}

			// 数据
			else if (token == "is?")
			{
				auto o1 = exec_stack.top();
				exec_stack.pop();
				auto o2 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createBool(is_(o1, o2)));
			}
			else if (token == "equal?")
			{
				auto o1 = exec_stack.top();
				exec_stack.pop();
				auto o2 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createBool(equal_(o1, o2)));
			}
			else if (token == "typeof")
			{
				EsmelObject target = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createString(type_of(target)));
			}
			else if (token == "set")
			{
				EsmelObject var_name = exec_stack.top();
				exec_stack.pop();
				EsmelObject val = exec_stack.top();
				exec_stack.pop();
				stack_frame.top().variables[getString(var_name)] = val;
			}

			// 逻辑
			else if (token == "not" || token == "!")
			{
				EsmelObject o1 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createBool(!bool_store_[o1.index]));
			}

			// 基本IO
			else if (token == "print")
			{
				std::cout << to_string(exec_stack.top());
				exec_stack.pop();
			}
			else if (token == "println")
			{
				std::cout << to_string(exec_stack.top()) << std::endl;
				exec_stack.pop();
			}

			// 数学
			else if (token == "add")
			{
				auto a1 = exec_stack.top();
				exec_stack.pop();
				auto a2 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createInt(int_store_[a1.index] + int_store_[a2.index]));
			}
			else if (token == "mul")
			{
				auto a1 = exec_stack.top();
				exec_stack.pop();
				auto a2 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createInt(int_store_[a1.index] * int_store_[a2.index]));
			}
			else if (token == "sub")
			{
				auto a1 = exec_stack.top();
				exec_stack.pop();
				auto a2 = exec_stack.top();
				exec_stack.pop();
				exec_stack.push(createInt(int_store_[a1.index] - int_store_[a2.index]));
			}


			// 其他自定义Esmel函数
			else if (esmel_functions.find(token) != esmel_functions.end())
			{
				call(token);
			}

			// 非函数
			else
			{
				EsmelObject e = parse_object(token);
				exec_stack.push(e);
			}
		}
		return line + 1;
	}

	string check_redefinition_safe(string &target, int line)
	{
		if (keywords.find(target) != keywords.end())
		{
			std::cerr << "Error: redefined token \"" << target << "\" at line " << line;
			exit(EXIT_FAILURE);
		}
		return target;
	}

	string check_redefinition_safe_and_add(string &target, int line)
	{
		string c = check_redefinition_safe(target, line);
		keywords.insert(target);
		return c;
	}

public:
	void run(string &filename, string &mainfunc)
	{
		// 解析时
		auto parsed = splitLinesFromFile(filename);
		string fn_name = "main";
		for (int i = 0; i < parsed.size(); i++)
		{
			auto code = parsed[i];
			if (code.empty()) continue;
			if (code[0] == "fn")
			{
				if (code.size() < 2)
				{
					std::cerr << "Error: empty function(fn) defined at line " << real_line_num[i] << std::endl;
					exit(0);
				}
				fn_name = check_redefinition_safe_and_add(code[1], i + 1);
				vector<string> args;
				for (int j = 2; j < code.size(); j++)
				{
					args.push_back(check_redefinition_safe(code[j], i + 1));
				}
				int cnt = real_line_num.size();
				esmel_functions[fn_name] = esmel_function{.arguments = args, .start = cnt, .end = cnt};
			}
			else if (code[0] == "flag")
			{
				if (code.size() != 2)
				{
					std::cerr << "Error: Illegal flag defined at line " << real_line_num[i] << std::endl;
					exit(0);
				}
				flags[check_redefinition_safe_and_add(code[1], i + 1)] = real_line_num.size();
			}
			else
			{
				real_line_num.push_back(i + 1);
				codes.push_back(code);
				esmel_functions[fn_name].end = real_line_num.size();
			}
		}
		call(mainfunc);
	}

	void run(string &filename)
	{
		string mainfunc = "main";
		run(filename, mainfunc);
	}
};
