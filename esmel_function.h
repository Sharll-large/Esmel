//
// Created by Sharll on 2025/11/2.
//

#pragma once

#include <vector>
#include <string>


enum class Type { INT, DOUBLE, BOOL, STRING, ARRAY, NULL_T };


struct esmel_function
{
	std::vector<std::string> arguments;
	// start和end表示这个函数包含的代码在有效代码vector中的下标。
	int start{};
	int end{};
};


struct EsmelObject
{
	Type type;
	size_t index; // 在对应类型数组中的索引
	int ref_count = 1; // 引用计数
};