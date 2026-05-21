#pragma once

#include <string>

enum class TaskStatus {
    TODO,
    IN_PROGRESS,
    DONE,
};

enum class TaskPriority {
    LOW,
    MEDIUM,
    HIGH,
};

struct Task {
    int id;
    std::string title;
    std::string description;
    TaskStatus status;
    TaskPriority priority;
    std::string created_at;
};
