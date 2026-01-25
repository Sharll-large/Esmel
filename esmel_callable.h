#pragma once

#include <vector>
#include <string>

#include "esmel_object.h"

enum class operation {
	CreateInt,
	CreateFloat,
	CreateBoolean,
	GetStaticStr,
	CreateUndefined,
	CreateType,
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
	NewArray, SetAt, GetAt, Append, GetLength, Link,
};

const std::unordered_map<std::string, Type> types = {
	{"Int", Type::INT},
	{"Float", Type::FLOAT},
	{"Boolean", Type::BOOLEAN},
	{"String", Type::STRING},
	{"Array", Type::ARRAY},
	{"UndefinedType", Type::UNDEFINED},
	{"Type", Type::TYPE}
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
	{"TypeOf", operation::Typeof},
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

const std::unordered_map<std::string, std::string> invalid = {
	{"int", "Int"}, {"float", "Float"}, {"boolean", "Boolean"}, {"string", "String"},{"array", "Array"}, {"undefined", "Undefined"},
	{"add", "Add"}, {"sub", "Sub"}, {"mul", "Mul"}, {"div", "Div"}, {"mod", "Mod"},
	{"set", "Set"}, {"if", "If"}, {"return", "Return"},
	{"goto", "a Jump, but you don't need this \'goto\' function before the flag name."},
{"Goto", "a Jump, but you don't need this \'goto\' function before the flag name."},
	{"flag", "Flag"}
};

struct esmel_op_code {
	operation op;
	uint64_t data;
};



class esmel_function {
public:
	// 实际信息
	uint64_t arguments;		// 参数长度
	uint64_t variable_count;
	std::vector<std::vector<esmel_op_code>> code;				// Esmel代码
	// 调试信息
	std::string name;											// 函数名称
	std::string file_name;								// 位于的文件名
	std::vector<uint64_t> real_line_num;				// 真实行号
};