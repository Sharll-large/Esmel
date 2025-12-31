#pragma once

#include <string>
#include <vector>

struct EsmelObject;

enum class Type {
	UNDEFINED, INT, FLOAT, BOOLEAN, STRING, ARRAY
};

struct esmel_string {string v; bool gcflag;};
struct esmel_array {vector<EsmelObject> v; bool gcflag;};

struct EsmelObject {
	Type type;						// 类型标记
	bool changeable = true; // 是否可变（用于常用对象缓存）
	union {
		long long int_v;
		double float_v;
		bool boolean_v;
		esmel_string* string_v;
		esmel_array* array_v;
	} value{};

	EsmelObject(Type t = Type::UNDEFINED): type(t) {

	}
	~EsmelObject() {}

	std::string to_string() {
		switch (type) {
		case Type::INT: return std::to_string(value.int_v);
		case Type::FLOAT: return std::to_string(value.float_v);
		case Type::BOOLEAN: return value.boolean_v ? "true" : "false";
		case Type::STRING: return value.string_v->v;
		case Type::ARRAY: {
			std::string result = "[";
			for (size_t i = 0; i < value.array_v->v.size(); ++i)
			{
				result += value.array_v->v.at(i).to_string();
				if (i < value.array_v->v.size() - 1)
				{
					result += ", ";
				}
			}
			result += "]";
			return result;
		}
		case Type::UNDEFINED: {
			return "undefined";
		}
		}
	}

	std::string type_of()
	{
		switch (type)
		{
		case Type::INT: return "int";
		case Type::FLOAT: return "float";
		case Type::BOOLEAN: return "bool";
		case Type::STRING: return "string";
		case Type::ARRAY: return "array";
		case Type::UNDEFINED: return "undefined";
		}
	}

	bool equal_to(const EsmelObject& another) {
		if (type != another.type) return false;
		switch (type)
		{
		case Type::INT: return value.int_v == another.value.int_v;
		case Type::FLOAT: return value.float_v == another.value.float_v;
		case Type::BOOLEAN: return value.boolean_v == another.value.boolean_v;
		case Type::STRING: return value.string_v->v == another.value.string_v->v;
		case Type::ARRAY:
		{
			if (value.array_v->v.size() != another.value.array_v->v.size()) return false;
			for (int i = 0; i < value.array_v->v.size(); i++)
			{
				if (!value.array_v->v.at(i).equal_to(another.value.array_v->v.at(i))) return false;
			}
			return true;
		}
		case Type::UNDEFINED: return true;
		}
	}

	bool is_same(const EsmelObject& another) {
		if (type != another.type) return false;

		switch (type) {
		case Type::STRING: return value.string_v == another.value.string_v;
		case Type::ARRAY: return value.array_v == another.value.array_v;
		default: {
			return equal_to(another);
		}
		}
	}

	EsmelObject(long long val) : type(Type::INT) {
		value.int_v = val;
	}

	EsmelObject(double val) : type(Type::FLOAT) {
		value.float_v = val;
	}

	EsmelObject(bool val) : type(Type::BOOLEAN) {
		value.boolean_v = val;
	}

	EsmelObject(esmel_string* val) : type(Type::STRING) {
		value.string_v = val;
	}

	EsmelObject(esmel_array* val) : type(Type::ARRAY) {
		value.array_v = val;
	}
};
