#pragma once

#include <string>
#include <nlohmann/json.hpp>

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

// --- JSON Mapping ---

NLOHMANN_JSON_SERIALIZE_ENUM(TaskStatus, {
    {TaskStatus::TODO, "TODO"},
    {TaskStatus::IN_PROGRESS, "IN_PROGRESS"},
    {TaskStatus::DONE, "DONE"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(TaskPriority, {
    {TaskPriority::LOW, "LOW"},
    {TaskPriority::MEDIUM, "MEDIUM"},
    {TaskPriority::HIGH, "HIGH"}
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Task, id, title, description, status, priority, created_at)
