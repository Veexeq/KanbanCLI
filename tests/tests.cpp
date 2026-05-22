#include <catch2/catch_test_macros.hpp>
#include "../src/KanbanBoard.hpp"
#include "../src/StorageManager.hpp"
#include <fstream>

TEST_CASE("KanbanBoard core operations", "[board]") {
    KanbanBoard board;

    SECTION("Adding tasks increments ID and sets default status") {
        board.addTask("Fix CI/CD", "Fix the compilation error", TaskPriority::HIGH);
        board.addTask("Write tests", "Write unit tests using Catch2", TaskPriority::MEDIUM);

        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        
        REQUIRE(todo_tasks.size() == 2);
        
        REQUIRE(todo_tasks[0].id == 1);
        REQUIRE(todo_tasks[0].title == "Fix CI/CD");
        REQUIRE(todo_tasks[0].priority == TaskPriority::HIGH);
        
        REQUIRE(todo_tasks[1].id == 2);
    }

    SECTION("Updating task status moves it to another column") {
        board.addTask("Task to move", "Description", TaskPriority::LOW);
        
        auto update_success = board.updateTaskStatus(1, TaskStatus::IN_PROGRESS);
        
        REQUIRE(update_success == true);
        REQUIRE(board.getTasksByStatus(TaskStatus::TODO).empty());
        REQUIRE(board.getTasksByStatus(TaskStatus::IN_PROGRESS).size() == 1);
    }

    SECTION("Removing a task deletes it from the board") {
        board.addTask("Task to delete", "Description", TaskPriority::LOW);
        
        auto remove_success = board.removeTask(1);
        
        REQUIRE(remove_success == true);
        REQUIRE(board.getTasksByStatus(TaskStatus::TODO).empty());
        
        REQUIRE(board.removeTask(999) == false);
    }
}

TEST_CASE("StorageManager persistence", "[storage]") {
    std::string test_file = "test_kanban.json";
    StorageManager storage(test_file);
    KanbanBoard board;

    SECTION("Saving and loading tasks preserves data integrity") {
        board.addTask("Persistent Task", "Should survive restart", TaskPriority::MEDIUM);
        auto original_tasks = board.getTasksByStatus(TaskStatus::TODO);

        auto save_success = storage.saveTasks(original_tasks);
        REQUIRE(save_success == true);

        auto loaded_tasks = storage.loadTasks();
        REQUIRE(loaded_tasks.size() == 1);
        REQUIRE(loaded_tasks[0].title == "Persistent Task");
        REQUIRE(loaded_tasks[0].status == TaskStatus::TODO);

        std::remove(test_file.c_str());
    }
}
