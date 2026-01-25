#pragma once

#include <string>
#include <vector>

#include "esmel_object.h"


class EsmelObjectPool {
    // 所有对象
    std::vector<esmel_string*> all_strings;
    std::vector<esmel_array*> all_arrays;

public:
    // 创建对象并添加到池中
    EsmelObject createString(const std::string& val) {
        auto* s = new esmel_string(val);
        all_strings.push_back(s);
        return {s};
    }

    EsmelObject createArray() {
        auto* obj = new esmel_array();
        all_arrays.push_back(obj);
        return {obj};
    }

    // 递归标记
    static void mark(const EsmelObject& obj) {
        switch (obj.type) {
        case Type::STRING:
            obj.value.string_v->marked = true;
            break;
        case Type::ARRAY: {
            // 数组则递归标记
            obj.value.array_v->marked = true;
            for (const auto& elem: obj.value.array_v->v) {
                mark(elem);
            }
            break;
        }
        default:
            break;
        }
    }

    // 清除未标记对象
    void gc() {
        uint64_t deleted = 0;
        for (uint64_t i = 0; i < all_strings.size(); i++) {
            if (all_strings[i]->marked) {
                all_strings[i]->marked = false;
                all_strings[i-deleted] = all_strings[i];
            } else {
                delete all_strings[i];
                deleted++;
            }
        }
        all_strings.resize(all_strings.size() - deleted);
        deleted = 0;
        for (uint64_t i = 0; i < all_arrays.size(); i++) {
            if (all_arrays[i]->marked) {
                all_arrays[i]->marked = false;
                all_arrays[i-deleted] = all_arrays[i];
            } else {
                delete all_arrays[i];
                deleted++;
            }
        }
        all_arrays.resize(all_arrays.size() - deleted);
    }
};
