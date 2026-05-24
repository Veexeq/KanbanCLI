#include "KanbanBoard.hpp"

#include <chrono>
#include <format>

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%Y-%m-%d %H:%M:%S}", now);
}

void KanbanBoard::addTask(const std::string& title, const std::string& description, TaskPriority priority) {
    Task new_task;
    new_task.id = m_next_id++;

    new_task.title = title;
    new_task.description = description;
    new_task.priority = priority;
    new_task.status = TaskStatus::TODO;
    new_task.created_at = getCurrentDateTime();

    m_tasks.push_back(new_task);
}

bool KanbanBoard::removeTask(int id) {
    for (size_t i = 0; i < m_tasks.size(); ++i) {
        
        if (m_tasks[i].id == id) {
            m_tasks.erase(m_tasks.begin() + i);
            return true;
        }
    }
    
    return false;
}

std::vector<Task> KanbanBoard::getTasksByStatus(TaskStatus status) const {
    std::vector<Task> filtered_tasks;

    for (const auto& task : m_tasks) {
        if (task.status == status) {
            filtered_tasks.push_back(task);
        }
    }

    return filtered_tasks;
}

bool KanbanBoard::updateTaskStatus(int id, TaskStatus new_status) {
    for (auto& task : m_tasks) {
        if (task.id == id) {
            task.status = new_status;
            return true;
        }
    }

    return false;
}

void KanbanBoard::updateTaskDetails(int id, const std::string& new_title, const std::string& new_description) {
    for (auto& task : m_tasks) {
        if (task.id == id) {
            task.title = new_title;
            task.description = new_description;
            break;
        }
    }
}
