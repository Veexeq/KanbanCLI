#pragma once

#include <string>
#include <vector>
#include "Task.hpp"

class KanbanBoard {
private:
    std::vector<Task> m_tasks;
    int m_next_id{1};

public:
    void addTask(const std::string& title, const std::string& description, TaskPriority priority);
    
    bool removeTask(int id);
    
    std::vector<Task> getTasksByStatus(TaskStatus status) const;

    bool updateTaskStatus(int id, TaskStatus new_status);

    void updateTaskDetails(int id, const std::string& new_title, const std::string& new_description);
};
