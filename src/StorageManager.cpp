#include "StorageManager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Alias for the scope resolution operator
using json = nlohmann::json;

StorageManager::StorageManager(const std::string& filename) : m_filename(filename) {}

bool StorageManager::saveTasks(const std::vector<Task>& tasks) const {
    // Open an output file stream (write)
    std::ofstream file(m_filename);
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open file for writing: " << m_filename << std::endl;
        return false;
    }

    // vector<Task> -> JSON oneliner, due to mappings in Task.hpp
    json json_data = tasks;

    // Write JSON to file, 4 = pretty print, for human readability
    file << json_data.dump(4);
    
    return true;
}

std::vector<Task> StorageManager::loadTasks() const {
    std::vector<Task> tasks;

    // Open an input file stream (read)
    std::ifstream file(m_filename);
    if (!file.is_open()) {
        return tasks; 
    }

    try {
        json json_data;
        file >> json_data;
        tasks = json_data.get<std::vector<Task>>();
    } 
    catch (const json::parse_error& e) {
        std::cerr << "[Warning] kanban.json is corrupted. Error: " << e.what() << std::endl;
        std::cerr << "[Warning] Starting with an empty board to prevent crash." << std::endl;
    }

    return tasks;
}
