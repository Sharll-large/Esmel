#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include <iostream>

#include "esmel_object.h"

class EsmelObjectPool {
private:
    // 所有对象
    std::vector<EsmelObject*> all_objects;


public:
    // 创建对象并添加到池中
    EsmelObject* createInt(long long val) {
        EsmelObject* obj = new EsmelObject(val);
        all_objects.push_back(obj);
        return obj;
    }

    EsmelObject* createFloat(double val) {
        EsmelObject* obj = new EsmelObject(val);
        all_objects.push_back(obj);
        return obj;
    }

    EsmelObject* createBoolean(bool val) {
        EsmelObject* obj = new EsmelObject(val);
        all_objects.push_back(obj);
        return obj;
    }

    EsmelObject* createString(const std::string& val) {
        EsmelObject* obj = new EsmelObject(val);
        all_objects.push_back(obj);
        return obj;
    }

    EsmelObject* createArray() {
        EsmelObject* obj = new EsmelObject(Type::ARRAY);
        all_objects.push_back(obj);
        return obj;
    }

    EsmelObject* createUndefined() {
        EsmelObject* obj = new EsmelObject(Type::UNDEFINED);
        all_objects.push_back(obj);
        return obj;
    }

    // 检查对象是否在池中
    bool contains(EsmelObject* obj) const {
        for (auto j: all_objects) {
            if (obj == j) return true;
        }
        return false;
    }


    ~EsmelObjectPool() {
        // 清理所有对象
        for (auto obj : all_objects) {
            delete obj;
        }
        all_objects.clear();
    }

    // 递归标记
    void mark(EsmelObject* obj) {
        if (!obj || obj->gc_mark) return;

        obj->gc_mark = true;

        // 如果是数组，递归标记所有元素
        if (obj->type == Type::ARRAY) {
            for (auto elem : *obj->value.array_v) {
                // 确保元素在池中
                if (contains(elem)) {
                    mark(elem);
                }
            }
        }
    }

    // 清除未标记对象
    void gc() {
        long long deleted = 0;
        for (long long i = 0; i < all_objects.size(); i++) {
            if (all_objects[i]->gc_mark) {
                all_objects[i]->gc_mark = false;
                all_objects[i-deleted] = all_objects[i];
            } else {
                delete all_objects[i];
                deleted++;
            }
        }
        all_objects.resize(all_objects.size() - deleted);
    }
};
