#pragma once
#include <functional>
#include <queue>
#include "Vector.hpp"

struct task_data
{
    int m;
    vec3 eye_pos;
    vec3 dir;
    int depth;
    int ssp;
    int id;
};

struct task
{
    task_data data;
    std::function<void(task_data)> func;
};