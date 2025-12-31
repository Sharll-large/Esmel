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
	Copy, Typeof, Equal, Gc,
	Print, Println, Readln, Input,
	Goto, If, Return,
	And, Or, Not,
	Call, Error,
	GetTime,
	Less,
	ELess,
	Greater,
	EGreater,
	NewArray, SetAt, GetAt, Append, GetLength, Link
};

const std::unordered_map<std::string, operation> vari_only_builtin = {
	// 只能用于变量的操作，如Set Add等。用于编译时优化
	{"Add", operation::AddBy},
	{"Sub", operation::SubBy},
	{"Mul", operation::MulBy},
	{"Div", operation::DivBy},
	{"Mod", operation::ModBy},
	{"Set", operation::SetVar},
	{"Input", operation::Input}
};

const std::unordered_map<std::string, operation> builtin = {
	{"Print", operation::Print},
	{"Println", operation::Println},
	{"Readln", operation::Readln},
	{"If", operation::If},
	{"Error", operation::Error},
	{"Return", operation::Return},
	{"Gc", operation::Gc},
	// 复制算数
	{"+", operation::Add},
	{"-", operation::Sub},
	{"*", operation::Mul},
	{"/", operation::Sub},
	{"%", operation::Mod},
	{"CurrentTime", operation::GetTime},
	{"Typeof", operation::Typeof},
	{"&&", operation::And},
	{"||", operation::Or},
	{"!", operation::Not},
	{"Equal?", operation::Equal},
	{"==", operation::Equal},
	{"Less?", operation::Less},
	{"LessEqual?", operation::ELess},
	{"GreaterEqual?", operation::EGreater},
	{"<", operation::Less},
	{"Greater?", operation::Greater},
	{">", operation::Greater},
	{">=", operation::EGreater},
	{"<=", operation::ELess},
	{"Or", operation::Or},
	{"And", operation::And},
	{"Not", operation::Not},
	// 容器（字符串，数组等）
	{"NewArray", operation::NewArray},
	{"Put", operation::SetAt},
	{"Get", operation::GetAt},
	{"Append", operation::Append},
	{"Len", operation::GetLength},
	{"Link", operation::Link},
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
	size_t arguments;		// 参数长度
	size_t variable_count;
	std::vector<std::string> static_strs; // 字符串字面量
	std::vector<std::vector<esmel_op_code>> code;				// Esmel代码
};