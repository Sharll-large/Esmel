//
// Created by Sharll on 2025/11/2.
//

#pragma once

#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <utility>
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

#define small_id_limit 128

using std::vector, std::string, std::unordered_map, std::map, std::stack, std::shared_ptr,
		std::unordered_set;

class frame // 栈帧
{
public:
	esmel_function* function;
	std::array<EsmelObject*, small_id_limit> temp_local_variables; // 小id变量直接存储
	unordered_map<size_t, EsmelObject*> local_variables; // 大id变量存入哈希表
	std::vector<EsmelObject*> exec_stack; // 执行栈
	int on_line;
	frame(esmel_function* f, unordered_map<size_t, EsmelObject*> vars): function(f), local_variables(std::move(vars)), on_line(0) {}
};

class EsmelInterpreter
{
public:
	EsmelObjectPool objects; // 对象池
	vector<esmel_function> functions; // 函数池
	std::vector<frame> stack_frame; // 栈帧（顶部表示当前的栈帧，存储局部变量信息。）

	void gc() {
		for (auto i: stack_frame) {
			for (auto j: i.exec_stack) objects.mark(j);
			for (auto j: i.temp_local_variables) objects.mark(j);
			for (auto j: i.local_variables) objects.mark(j.second);
		}
		objects.gc();
	}


	void call(long long id)
	// 调用一个非内置的esmel函数。
	{
		auto nframe = frame(&functions[id], {});

		for (auto& i: nframe.temp_local_variables) i = objects.createUndefined();

		for (int i = 0; i < nframe.function->arguments; i++)
		{
			if (stack_frame.back().exec_stack.empty())
			{
				std::cerr << "Error: needs " << nframe.function->arguments << " argument(s), but only " << i << " are given.";
				error();
			}
			if (i < small_id_limit) nframe.temp_local_variables[i] = stack_frame.back().exec_stack.back();
			else nframe.local_variables[i] = stack_frame.back().exec_stack.back();
			stack_frame.back().exec_stack.pop_back();
		}

		stack_frame.push_back(nframe);

		int& l = stack_frame.back().on_line;
		while (l < stack_frame.back().function->code.size()) {
			l = exec_line(stack_frame.back().function->code[l], l);
		}
	}

	void error()
	// 打印调用栈并非正常退出。
	{
		while (!stack_frame.empty())
		{
			auto st = stack_frame.back();
			std::cerr << std::endl << "\tat " << st.function->file_name << '(' << st.function->real_line_num[st.on_line] << ")";
			stack_frame.pop_back();
		}
		exit(EXIT_FAILURE);
	}
	int exec_line(vector<esmel_op_code> &code, int line)
	// 执行一段esmel代码
	{

		for (long long i = code.size() - 1; i >= 0; i--)
		{
			frame& current_frame = stack_frame.back();
			vector<EsmelObject*>& current_stack = current_frame.exec_stack;
			size_t size = current_stack.size();
			auto token = code[i];
			switch (token.op) {
			case operation::CreateInt:
				current_stack.push_back(objects.createInt(token.data));
				break;
			case operation::CreateFloat:
				current_stack.push_back(objects.createFloat(std::bit_cast<double>(token.data)));
				break;
			case operation::CreateBoolean:
				current_stack.push_back(objects.createBoolean(token.data));
				break;
			case operation::GetStaticStr:
				current_stack.push_back(objects.createString(stack_frame.back().function->static_strs[token.data]));
				break;
			case operation::CreateUndefined:
				current_stack.push_back(objects.createUndefined());
				break;
			case operation::GetVar: {
				if (token.data < 128) {
					current_stack.push_back(current_frame.temp_local_variables[token.data]);
					break;
				}else{
					auto k = current_frame.local_variables.find(token.data);
					if (k == current_frame.local_variables.end()) {
						current_frame.local_variables[token.data] = objects.createUndefined();
					}
					current_stack.push_back(current_frame.local_variables[token.data]);
					break;
				}
			}
			case operation::SetVar: {
				EsmelObject* source = current_stack[size-2];
				EsmelObject* target = current_stack[size-1];
				current_stack.resize(size-2);
				*target = *source;
				break;
			}
			case operation::Add:
			{
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for add: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					current_stack.push_back(objects.createInt(a1->value.int_v + a2->value.int_v));
					break;
				case Type::FLOAT:
					current_stack.push_back(objects.createFloat(a1->value.float_v + a2->value.float_v));
					break;
				case Type::STRING:
					current_stack.push_back(objects.createString(*a1->value.string_v + *a2->value.string_v));
					break;
				case Type::ARRAY:
					break;
				default: {
					cerr << "Unsupported type for add: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::Sub: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for sub: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					current_stack.push_back(objects.createInt(a1->value.int_v - a2->value.int_v));
					break;
				case Type::FLOAT:
					current_stack.push_back(objects.createFloat(a1->value.float_v - a2->value.float_v));
					break;
				default: {
					cerr << "Unsupported type for sub: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}

			case operation::Mul: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for mul: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					current_stack.push_back(objects.createInt(a1->value.int_v * a2->value.int_v));
					break;
				case Type::FLOAT:
					current_stack.push_back(objects.createFloat(a1->value.float_v * a2->value.float_v));
					break;
				default: {
					cerr << "Unsupported type for mul: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::Div: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for div: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					current_stack.push_back(objects.createInt(a1->value.int_v / a2->value.int_v));
					break;
				case Type::FLOAT:
					current_stack.push_back(objects.createFloat(a1->value.float_v / a2->value.float_v));
					break;
				default: {
					cerr << "Unsupported type for div: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::Mod: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for mod: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					current_stack.push_back(objects.createInt(a1->value.int_v % a2->value.int_v));
					break;
				default: {
					cerr << "Unsupported type for mod: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::Print:
				std::cout << current_stack.back()->to_string();
				current_stack.pop_back();
				break;
			case operation::Goto:
				return token.data;
			case operation::Call:
				call(token.data);
				break;
			case operation::AddBy: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for AddBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					a1->value.int_v += a2->value.int_v;
					break;
				case Type::FLOAT:
					a1->value.float_v += a2->value.float_v;
					break;
				case Type::STRING:
					*a1->value.string_v += *a2->value.string_v;
					break;
				case Type::ARRAY:
					break;
				default: {
					cerr << "Unsupported type for AddBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::SubBy: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for SubBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					a1->value.int_v -= a2->value.int_v;
					break;
				case Type::FLOAT:
					a1->value.float_v -= a2->value.float_v;
					break;
				default: {
					cerr << "Unsupported type for SubBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::MulBy: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for MulBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					a1->value.int_v *= a2->value.int_v;
					break;
				case Type::FLOAT:
					a1->value.float_v *= a2->value.float_v;
					break;
				default: {
					cerr << "Unsupported type for MulBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::DivBy: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for DivBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					a1->value.int_v /= a2->value.int_v;
					break;
				case Type::FLOAT:
					a1->value.float_v /= a2->value.float_v;
					break;
				default: {
					cerr << "Unsupported type for DivBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::ModBy: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type != a2->type) {
					cerr << "Unsupported type for ModBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				switch (a1->type) {
				case Type::INT:
					a1->value.int_v %= a2->value.int_v;
					break;
				default: {
					cerr << "Unsupported type for ModBy: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				}
				break;
			}
			case operation::Copy:
				break;
			case operation::Typeof: {
				EsmelObject* a1 = current_stack[size-1];
				current_stack[size-1] = objects.createString(a1->type_of());
				current_stack.resize(size-1);
				break;
			}
			case operation::Is: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-1);
				current_stack[size-2] = objects.createBoolean(a1 == a2);
				break;
			}
			case operation::Equal: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-1);
				current_stack[size-2] = objects.createBoolean(a1->equal_to(*a2));
				break;
			}
			case operation::Gc:
				gc();
				break;
			case operation::Println:
				std::cout << current_stack.back()->to_string() << std::endl;
				current_stack.pop_back();
				break;
			case operation::If: {
				const auto condition = stack_frame.back().exec_stack.back();stack_frame.back().exec_stack.pop_back();
				if (condition->type != Type::BOOLEAN) {
					cerr << "\'if\' must take a boolean value, but get: " << condition->type_of();
					error();
				}
				if (!condition->value.boolean_v) return line+1;
				break;
			}
			case operation::Return: {
				const auto thing = current_stack.back();
				auto ln = current_frame.function->code.size();
				stack_frame.pop_back();
				stack_frame.back().exec_stack.push_back(thing);
				return ln+1;
				break;
			}
			case operation::And: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type == a2->type && a1->type == Type::BOOLEAN) {
					current_stack[size-2] = objects.createBoolean(a1->value.boolean_v && a2->value.boolean_v);
				} else {
					cerr << "Unsupported type for and: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				break;
			}
			case operation::Or: {
				EsmelObject* a2 = current_stack[size-2];
				EsmelObject* a1 = current_stack[size-1];
				current_stack.resize(size-2);
				if (a1->type == a2->type && a1->type == Type::BOOLEAN) {
					current_stack[size-2] = objects.createBoolean(a1->value.boolean_v || a2->value.boolean_v);
				} else {
					cerr << "Unsupported type for or: " << a1->type_of() << " and " << a2->type_of();
					error();
				}
				break;
			}
			case operation::Not: {
				EsmelObject* a1 = current_stack[size-1];
				if (a1->type == Type::BOOLEAN) {
					current_stack[size-1] = objects.createBoolean(a1->value.boolean_v);
					current_stack.resize(size-1);
				} else {
					cerr << "Unsupported type for not: " << a1->type_of();
				}
				break;
			}
			case operation::Error:
				error();
				break;
			case operation::GetTime:
				stack_frame.back().exec_stack.push_back(objects.createInt(
				std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
				));
				break;
			default:
				break;
			}
		}
		stack_frame.back().exec_stack.clear();
		return line + 1;
	}
};
