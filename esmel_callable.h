#pragma once

#include <vector>
#include <string>

enum class operation {
	CreateInt,
	CreateFloat,
	CreateBoolean,
	GetStaticStr,
	CreateUndefined,
	GetVar, SetVar,
	Add, Sub, Mul, Div, Mod,
	AddBy, SubBy, MulBy, DivBy, ModBy,
	Copy, Typeof, Is, Equal, Gc,
	Print, Println,
	Goto, If, Return,
	And, Or, Not,
	Call, Error,
	GetTime,
	Less,
};

const std::unordered_map<std::string, operation> builtin = {
	{"print", operation::Print},
	{"println", operation::Println},
	{"set", operation::SetVar},
	{"if", operation::If},
{"equal?", operation::Equal},
	{"error", operation::Error},
	{"return", operation::Return},
	{"gc", operation::Gc},
	// 原地算术
	{"add", operation::AddBy},
	{"sub", operation::SubBy},
	{"mul", operation::MulBy},
	{"div", operation::DivBy},
	{"mod", operation::ModBy},
	// 复制算数
	{"+", operation::Add},
	{"-", operation::Sub},
	{"*", operation::Mul},
	{"/", operation::Sub},
	{"%", operation::Mod},
	{"currentTime", operation::GetTime},
	{"typeof", operation::Typeof},
	{"is?", operation::Is},
	{"&&", operation::And},
		{"||", operation::Or},
	{"!", operation::Not},
	{"error", operation::Error}
};

struct esmel_op_code {
	operation op;
	int64_t data;
};



class esmel_function {
public:
	// 调试信息
	std::string file_name;								// 位于的文件名
	std::vector<int> real_line_num;				// 真实行号
	// 实际信息
	long long arguments;		// 参数长度
	std::vector<std::string> static_strs; // 字符串字面量
	std::vector<std::vector<esmel_op_code>> code;				// Esmel代码
};