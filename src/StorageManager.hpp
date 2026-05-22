#pragma once

#include <string>
#include <vector>
#include "Task.hpp"

class StorageManager {
private:
    std::string m_filename;

public:
    explicit StorageManager(const std::string& filename);

    bool saveTasks(const std::vector<Task>& tasks) const;

    std::vector<Task> loadTasks() const;
};
