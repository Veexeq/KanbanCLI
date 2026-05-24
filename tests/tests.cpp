/**
 * @file tests.cpp
 * @brief Unit testing suite verifying the core business logic mutations of the KanbanBoard.
 */

#include <catch2/catch_test_macros.hpp>
#include "../src/KanbanBoard.hpp"

TEST_CASE("KanbanBoard - Core State and Business Logic Mutations", "[kanban]") {
    KanbanBoard board;

    // A baseline task is appended to establish an isolated record tracking context.
    board.addTask("Original Title", "Original Description", TaskPriority::MEDIUM);
    
    // Domain model entities typically assign a sequence identifier of 1 to the initial record.
    int test_task_id = 1;

    SECTION("Task text and priority components can be successfully mutated via editing hooks") {
        // UPDATED: Execute the targeted business logic update method passing the new priority parameter
        board.updateTaskDetails(test_task_id, "Revised Title", "Revised Description", TaskPriority::HIGH);
        
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        
        // Assertions verify the data payloads have updated accurately inside the memory store.
        REQUIRE(todo_tasks.size() == 1);
        CHECK(todo_tasks[0].title == "Revised Title");
        CHECK(todo_tasks[0].description == "Revised Description");
        CHECK(todo_tasks[0].priority == TaskPriority::HIGH);
    }

    SECTION("Tasks transition forward and backward cleanly across workflow swimlanes") {
        // Operational Pass A: Forward workflow progression loop execution tracking.
        board.updateTaskStatus(test_task_id, TaskStatus::IN_PROGRESS);
        CHECK(board.getTasksByStatus(TaskStatus::IN_PROGRESS).size() == 1);
        CHECK(board.getTasksByStatus(TaskStatus::TODO).empty());

        board.updateTaskStatus(test_task_id, TaskStatus::DONE);
        CHECK(board.getTasksByStatus(TaskStatus::DONE).size() == 1);
        CHECK(board.getTasksByStatus(TaskStatus::IN_PROGRESS).empty());

        // Operational Pass B: Backward workflow regression loop execution tracking.
        board.updateTaskStatus(test_task_id, TaskStatus::IN_PROGRESS);
        CHECK(board.getTasksByStatus(TaskStatus::IN_PROGRESS).size() == 1);
        CHECK(board.getTasksByStatus(TaskStatus::DONE).empty());

        board.updateTaskStatus(test_task_id, TaskStatus::TODO);
        CHECK(board.getTasksByStatus(TaskStatus::TODO).size() == 1);
        CHECK(board.getTasksByStatus(TaskStatus::IN_PROGRESS).empty());
    }

    SECTION("Tasks can be forcefully purged and extracted from the board matrix repository") {
        // Execute the structural deletion mutation.
        board.removeTask(test_task_id);
        
        // Verify that the task collection array has successfully collapsed to zero.
        CHECK(board.getTasksByStatus(TaskStatus::TODO).empty());
    }
}
