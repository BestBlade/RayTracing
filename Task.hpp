#pragma once
#include "Vector.hpp"
#include <functional>
//  ���Ķ��߳��࣬���û�õ�
struct task_data {
    int m;
    int depth;
    int ssp;
    int id;
    vec3 eye_pos;
    vec3 dir;
};

struct task {
    task_data data;
    std::function<void(task_data)> func;
};