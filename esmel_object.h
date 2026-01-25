#pragma once

#include <list>
#include <string>
#include <vector>

struct EsmelObject;

enum class Type {
	UNDEFINED, INT, FLOAT, BOOLEAN, STRING, ARRAY, TYPE
};

struct esmel_string {std::string v; bool marked;};
struct esmel_array {std::vector<EsmelObject> v; bool marked;};
// struct esmel_map {unordered_map<EsmelObject, EsmelObject> v; list<EsmelObject> l; bool marked;};

struct EsmelObject {
	Type type;						// 类型标记
	union {
		int64_t int_v;
		double float_v;
		bool boolean_v;
		esmel_string* string_v;
		esmel_array* array_v;
		Type type_v;
	} value{};

	EsmelObject(): type(Type::UNDEFINED) {}
	~EsmelObject() = default;

	[[nodiscard]] std::string to_string() const {
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
		case Type::TYPE: {
			switch (value.type_v)
			{
			case Type::INT: return "<type:Int>";
			case Type::FLOAT: return "<type:Float>";
			case Type::BOOLEAN: return "<type:Boolean>";
			case Type::STRING: return "<type:String>";
			case Type::ARRAY: return "<type:Array>";
			case Type::UNDEFINED: return "<type:Undefined>";
			case Type::TYPE: return "<type:Type>";
			}
		}

		default:
			return "Unknown";
		}
	}

	[[nodiscard]] std::string type_of() const {
		return EsmelObject(type).to_string();
	}

	[[nodiscard]] bool equal_to(const EsmelObject& another) const {
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
			for (size_t i = 0; i < value.array_v->v.size(); i++)
			{
				if (!value.array_v->v.at(i).equal_to(another.value.array_v->v.at(i))) return false;
			}
			return true;
		}
		case Type::UNDEFINED: return true;
		case Type::TYPE: return value.type_v == another.value.type_v;
		}
		return false;
	}

	[[nodiscard]] bool is_same(const EsmelObject& another) const {
		if (type != another.type) return false;

		switch (type) {
		case Type::STRING: return value.string_v == another.value.string_v;
		case Type::ARRAY: return value.array_v == another.value.array_v;
		default: {
			return equal_to(another);
		}
		}
	}

	EsmelObject(int64_t val) : type(Type::INT) {
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
	EsmelObject(Type val) : type(Type::TYPE) {
		value.type_v = val;
	}
};
