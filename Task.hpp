#pragma once
#include "Vector.hpp"
#include <functional>
//  抄的多线程类，最后没用到
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