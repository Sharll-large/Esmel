//
// Created by Sharll on 2025/11/2.
//

#pragma once

#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <stack>
#include <memory>
#include <unordered_set>

#include "esmel_callable.h"
#include "esmel_object.h"
#include "esmel_gc.h"

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::shared_ptr,
		std::unordered_set;

class frame // 栈帧
{
public:
	esmel_function* function;
	std::vector<EsmelObject> local_variables; // 变量存储，id为索引
	std::vector<EsmelObject> exec_stack; // 执行栈
	uint64_t on_line;
	frame(esmel_function* f): function(f), on_line(0) {
		local_variables.resize(f->variable_count, {});
	}
};

class EsmelInterpreter
{
public:
	EsmelObjectPool objects; // 对象池
	vector<esmel_function> functions; // 函数池
	vector<std::string> static_str;		// 字符串字面量池
	std::vector<frame> stack_frame; // 栈帧（顶部表示当前的栈帧，存储局部变量信息。）

	void gc() {
		for (const auto& i: stack_frame) {
			for (const auto& j: i.exec_stack) objects.mark(j);
			for (const auto& j: i.local_variables) objects.mark(j);
		}
		objects.gc();
	}


	void call(const uint64_t id)
	// 调用一个非内置的esmel函数。
	{
		auto nframe = frame(&functions[id]);

		for (int i = 0; i < nframe.function->arguments; i++)
		{
			if (stack_frame.back().exec_stack.empty())
			{
				std::cerr << "Error: needs " << nframe.function->arguments << " argument(s), but only " << i << " are given.";
				error();
			}
			nframe.local_variables[i] = stack_frame.back().exec_stack.back();
			stack_frame.back().exec_stack.pop_back();
		}

		stack_frame.push_back(nframe);

		uint64_t& l = stack_frame.back().on_line;
		while (l < stack_frame.back().function->code.size()) {
			// std::cout << "Line " << l << ", func " << stack_frame.back().function->name << ":\t";
			l = exec_line(stack_frame.back().function->code[l], l);
		}
		if (stack_frame.size() == 1) {
			if (stack_frame.back().exec_stack.empty()) exit(0);
			exit(stack_frame.back().exec_stack.back().value.int_v);
		}
		if (stack_frame.back().exec_stack.empty()) {
			stack_frame.pop_back();
			stack_frame.back().exec_stack.emplace_back();
		} else {
			auto t = stack_frame.back().exec_stack.back();
			stack_frame.pop_back();
			stack_frame.back().exec_stack.push_back(t);
		}
	}

	void error()
	// 打印调用栈并非正常退出。
	{
		while (!stack_frame.empty())
		{
			auto st = stack_frame.back();
			std::cerr << std::endl << "\tat " << st.function->name << '(' << st.function->file_name << ':' << st.function->real_line_num[st.on_line] << ")";
			stack_frame.pop_back();
		}
		exit(EXIT_FAILURE);
	}

	uint64_t exec_line(vector<esmel_op_code> &code, uint64_t line)
	// 执行一段esmel代码
	{
		for (auto [op, data]: code)
		{
			frame& current_frame = stack_frame.back();
			vector<EsmelObject>& current_stack = current_frame.exec_stack;
			size_t size = current_stack.size();
			switch (op) {
			case operation::CreateInt:
				current_stack.emplace_back(std::bit_cast<int64_t>(data));
				break;
			case operation::CreateFloat:
				current_stack.emplace_back(std::bit_cast<double>(data));
				break;
			case operation::CreateBoolean:
				current_stack.emplace_back(static_cast<bool>(data));
				break;
			case operation::CreateType:
				current_stack.emplace_back(static_cast<Type>(data));
				break;
			case operation::GetStaticStr:
				current_stack.push_back(objects.createString(static_str[data]));
				break;
			case operation::CreateUndefined:
				current_stack.emplace_back();
				break;
			case operation::GetVar: {
				current_stack.push_back(current_frame.local_variables[data]);
				break;
			}
			case operation::SetVar: {
				current_frame.local_variables[data] = current_stack[size-1];
				current_stack.resize(size-1);
				break;
			}
			case operation::Add: {
				auto a1 = current_stack[size-1];
				auto a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported type for Add: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::INT:
					current_stack.emplace_back(a1.value.int_v + a2.value.int_v);
					break;
				case Type::FLOAT:
					current_stack.emplace_back(a1.value.float_v + a2.value.float_v);
					break;
				default: {
					cerr << "Unsupported type for Add: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}
			case operation::Sub: {
				auto a1 = current_stack[size-1];
				auto a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported type for Subtract: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::INT:
					current_stack.emplace_back(a1.value.int_v - a2.value.int_v);
					break;
				case Type::FLOAT:
					current_stack.emplace_back(a1.value.float_v - a2.value.float_v);
					break;
				default: {
					cerr << "Unsupported type for Subtract: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}

			case operation::Mul: {
				auto a1 = current_stack[size-1];
				auto a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported type for Multiply: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::INT:
					current_stack.emplace_back(a1.value.int_v * a2.value.int_v);
					break;
				case Type::FLOAT:
					current_stack.emplace_back(a1.value.float_v * a2.value.float_v);
					break;
				default: {
					cerr << "Unsupported type for Multiply: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}
			case operation::Div: {
				auto a1 = current_stack[size-1];
				auto a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported type for Division: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::INT:
					current_stack.emplace_back(a1.value.int_v / a2.value.int_v);
					break;
				case Type::FLOAT:
					current_stack.emplace_back(a1.value.float_v / a2.value.float_v);
					break;
				default: {
					cerr << "Unsupported type for Division: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}
			case operation::Mod: {
				auto a1 = current_stack[size-1];
				auto a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported type for Modulo: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::INT:
					current_stack.emplace_back(a1.value.int_v % a2.value.int_v);
					break;
				default: {
					cerr << "Unsupported type for Modulo: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}
			case operation::Print:
				std::cout << current_stack.back().to_string();
				current_stack.pop_back();
				break;
			case operation::Goto:
				return data;
			case operation::Call:
				call(data);
				break;
			case operation::AddBy: {
				auto& origin = current_frame.local_variables[data];
				auto a = current_stack[size-1];
				current_stack.resize(size-1);
				if (origin.type != a.type) {
					cerr << "Unsupported type for Add: " << origin.type_of() << " and " << a.type_of();
					error();
				}
				switch (a.type) {
				case Type::INT:
					origin.value.int_v += a.value.int_v;
					break;
				case Type::FLOAT:
					origin.value.float_v += a.value.float_v;;
					break;
				default: {
					cerr << "Unsupported type for Add: " << a.type_of() << " and " << a.type_of();
					error();
				}
				}
				break;
			}
			case operation::SubBy: {
				auto& origin = current_frame.local_variables[data];
				auto a = current_stack[size-1];
				current_stack.resize(size-1);
				if (origin.type != a.type) {
					cerr << "Unsupported type for Subtract: " << origin.type_of() << " and " << a.type_of();
					error();
				}
				switch (a.type) {
				case Type::INT:
					origin.value.int_v -= a.value.int_v;
					break;
				case Type::FLOAT:
					origin.value.float_v -= a.value.float_v;;
					break;
				default: {
					cerr << "Unsupported type for Subtract: " << a.type_of() << " and " << a.type_of();
					error();
				}
				}
				break;
			}
			case operation::MulBy: {
				auto& origin = current_frame.local_variables[data];
				auto a = current_stack[size-1];
				current_stack.resize(size-1);
				if (origin.type != a.type) {
					cerr << "Unsupported type for Multiply: " << origin.type_of() << " and " << a.type_of();
					error();
				}
				switch (a.type) {
				case Type::INT:
					origin.value.int_v *= a.value.int_v;
					break;
				case Type::FLOAT:
					origin.value.float_v *= a.value.float_v;;
					break;
				default: {
					cerr << "Unsupported type for Multiply: " << a.type_of() << " and " << a.type_of();
					error();
				}
				}
				break;
			}
			case operation::DivBy: {
				auto& origin = current_frame.local_variables[data];
				auto a = current_stack[size-1];
				current_stack.resize(size-1);
				if (origin.type != a.type) {
					cerr << "Unsupported type for Division: " << origin.type_of() << " and " << a.type_of();
					error();
				}
				switch (a.type) {
				case Type::INT:
					origin.value.int_v /= a.value.int_v;
					break;
				case Type::FLOAT:
					origin.value.float_v /= a.value.float_v;;
					break;
				default: {
					cerr << "Unsupported type for Division: " << a.type_of() << " and " << a.type_of();
					error();
				}
				}
				break;
			}
			case operation::ModBy: {
				auto& origin = current_frame.local_variables[data];
				auto a = current_stack[size-1];
				current_stack.resize(size-1);
				if (origin.type != a.type) {
					cerr << "Unsupported type for Modulo: " << origin.type_of() << " and " << a.type_of();
					error();
				}
				switch (a.type) {
				case Type::INT:
					origin.value.int_v %= a.value.int_v;
					break;
				default: {
					cerr << "Unsupported type for Modulo: " << a.type_of() << " and " << a.type_of();
					error();
				}
				}
				break;
			}
			case operation::Copy:
				break;
			case operation::Typeof: {
				current_stack[size-1] = EsmelObject(current_stack[size-1].type);
				break;
			}
			case operation::Equal: {
				EsmelObject a2 = current_stack[size-2];
				EsmelObject a1 = current_stack[size-1];
				current_stack[size-2] = {a1.equal_to(a2)};
				current_stack.resize(size-1);
				break;
			}
			case operation::Gc:
				gc();
				break;
			case operation::Println:
				std::cout << current_stack.back().to_string() << std::endl;
				current_stack.pop_back();
				break;
			case operation::If: {
				const auto condition = stack_frame.back().exec_stack.back();stack_frame.back().exec_stack.pop_back();
				if (condition.type != Type::BOOLEAN) {
					cerr << "\'if\' must take a boolean value, but get: " << condition.value.int_v;
					error();
				}
				if (!condition.value.boolean_v) return line+1;
				break;
			}
			case operation::Return: {
				return current_frame.function->code.size()+1;
			}
			case operation::And: {
				EsmelObject a2 = current_stack[size-2];
				EsmelObject a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1.type == a2.type && a1.type == Type::BOOLEAN) {
					current_stack[size-2] = a1.value.boolean_v && a2.value.boolean_v;
				} else {
					cerr << "Logic And must take two boolean types, but get: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				current_stack.resize(size-1);
				break;
			}
			case operation::Or: {
				EsmelObject a2 = current_stack[size-2];
				EsmelObject a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1.type == a2.type && a1.type == Type::BOOLEAN) {
					current_stack[size-2] = a1.value.boolean_v || a2.value.boolean_v;
				} else {
					cerr << "Logic Or must take two boolean types, but get: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				current_stack.resize(size-1);
				break;
			}
			case operation::Not: {
				EsmelObject a = current_stack[size-1];
				if (a.type == Type::BOOLEAN) {
					current_stack[size-1] = !a.value.boolean_v;
				} else {
					cerr << "Logic Not must take a boolean type, but get: " << a.type_of();
					error();
				}
				break;
			}
			case operation::Error: {
				EsmelObject a = current_stack[size-1];
				cerr << a.to_string();
				error();
				break;
			}
			case operation::GetTime: {
				current_stack.emplace_back(
				std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
				);
				break;
			}
			case operation::Readln: {
				auto o = objects.createString("");
				std::getline(std::cin, o.value.string_v->v);
				current_stack.push_back(o);
				break;
			}
			case operation::Less:
				break;
			case operation::ELess:
				break;
			case operation::Greater:
				break;
			case operation::EGreater:
				break;
			case operation::Input: {
				auto o = current_frame.local_variables[data] = objects.createString("");
				std::getline(std::cin, o.value.string_v->v);
				break;
			}
			case operation::NewArray: {
				current_stack.push_back(objects.createArray());
				break;
			}
			case operation::SetAt: {
				EsmelObject origin = current_stack[size-1];
				EsmelObject index = current_stack[size-2];
				EsmelObject target = current_stack[size-3];
				current_stack.resize(size-3);
				if (origin.type != Type::ARRAY) {
					cerr << "SetAt can only be used on arrays, but get: " << origin.type_of();
					error();
				}
				if (index.type != Type::INT) {
					cerr << "SetAt index must be an Integer, but getL " << index.type_of();
					error();
				}
				if (index.value.int_v >= origin.value.array_v->v.size() || index.value.int_v < 0) {
					cerr << "Index " << index.value.int_v << " out of range.";
					error();
				}
				origin.value.array_v->v[index.value.int_v] = target;
				break;
			}
			case operation::GetAt: {
				EsmelObject origin = current_stack[size-1];
				EsmelObject index = current_stack[size-2];
				current_stack.resize(size-1);
				if (origin.type != Type::ARRAY) {
					cerr << "SetAt can only be used on arrays, but get: " << origin.type_of();
					error();
				}
				if (index.type != Type::INT) {
					cerr << "SetAt index must be an Integer, but getL " << index.type_of();
					error();
				}
				if (index.value.int_v >= origin.value.array_v->v.size() || index.value.int_v < 0) {
					cerr << "Index " << index.value.int_v << " out of range.";
					error();
				}
				current_stack[size-2] = origin.value.array_v->v[index.value.int_v];
				break;
			}
			case operation::Append: {
				EsmelObject origin = current_stack[size-1];
				EsmelObject target = current_stack[size-2];
				current_stack.resize(size-2);
				if (origin.type != Type::ARRAY) {
					cerr << "SetAt can only be used on arrays, but get: " << origin.type_of();
					error();
				}
				origin.value.array_v->v.push_back(target);
				break;
			}
			case operation::GetLength:
				if (current_stack[size-1].type == Type::ARRAY) {
					current_stack[size-1] = EsmelObject{static_cast<long long>(current_stack[size - 1].value.array_v->v.size())};
				} else if (current_stack[size-1].type == Type::STRING) {
					current_stack[size-1] = EsmelObject{static_cast<long long>(current_stack[size - 1].value.string_v->v.size())};
				} else {
					cerr << "Unsupported types for Len: " << current_stack[size-1].type_of();
					error();
				}
				break;
			case operation::Link: {
				EsmelObject a1 = current_stack[size-1];
				EsmelObject a2 = current_stack[size-2];
				current_stack.resize(size-2);
				if (a1.type != a2.type) {
					cerr << "Unsupported types for Link: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				switch (a1.type) {
				case Type::STRING: {
					current_stack.push_back(objects.createString(a1.value.string_v->v + a2.value.string_v->v));
					break;
				}
				case Type::ARRAY: {
					auto a = objects.createArray();
					a.value.array_v->v.reserve(a1.value.array_v->v.size() + a2.value.array_v->v.size());
					std::ranges::copy(a1.value.array_v->v, std::back_inserter(a.value.array_v->v));
					std::ranges::copy(a2.value.array_v->v, std::back_inserter(a.value.array_v->v));
					current_stack.push_back(a);
					break;
				}
				default: {
					cerr << "Unsupported types for Link: " << a1.type_of() << " and " << a2.type_of();
					error();
				}
				}
				break;
			}
			default:
				break;
			}
		}
		stack_frame.back().exec_stack.clear();
		// cout << "Return " << line+1 << endl;
		return line + 1;
	}
};
