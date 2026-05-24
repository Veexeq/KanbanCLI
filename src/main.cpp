#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include "KanbanBoard.hpp"
#include "StorageManager.hpp"

// A helper function is declared to serialize the entire board state to the persistent layer.
void saveBoardState(const KanbanBoard& board, const StorageManager& storage) {
    std::vector<::Task> all_tasks;
    
    // Tasks are extracted from all structural columns sequentially.
    for (auto status : {TaskStatus::TODO, TaskStatus::IN_PROGRESS, TaskStatus::DONE}) {
        auto column_tasks = board.getTasksByStatus(status);
        all_tasks.insert(all_tasks.end(), column_tasks.begin(), column_tasks.end());
    }
    
    // The compiled vector is written out to the JSON file.
    storage.saveTasks(all_tasks);
}

int main() {
    using namespace ftxui;

    // The storage engine and core domain model container are instantiated.
    StorageManager storage("kanban.json");
    KanbanBoard board;

    // Persistent data is loaded from disk if the storage target exists.
    auto loaded_tasks = storage.loadTasks();
    if (!loaded_tasks.empty()) {
        for (const auto& task : loaded_tasks) {
            board.addTask(task.title, task.description, task.priority);
            if (task.status != TaskStatus::TODO) {
                auto active_tasks = board.getTasksByStatus(TaskStatus::TODO);
                if (!active_tasks.empty()) {
                    board.updateTaskStatus(active_tasks.back().id, task.status);
                }
            }
        }
    } else {
        board.addTask("Write presentation", "Prepare slides for university demo", TaskPriority::HIGH);
        board.addTask("Refactor code", "Clean up variable names and enforce consistency", TaskPriority::MEDIUM);
        board.addTask("Buy coffee", "Crucial resource for long programming sessions", TaskPriority::LOW);
        board.updateTaskStatus(2, TaskStatus::IN_PROGRESS);
        saveBoardState(board, storage);
    }

    // Coordinate state trackers for the grid matrix are declared.
    int selected_column = 0;       
    int selected_task_index = 0;   

    // Modal view tracking variables and input buffers are established.
    bool is_modal_active = false;
    bool is_edit_mode = false;      // New state tracker to distinguish creation from modification
    int editing_task_id = -1;       // Stores the active ID of the entity being mutated
    
    std::string input_title_buffer;
    std::string input_desc_buffer;

    Component input_title = Input(&input_title_buffer, "Enter task title...");
    Component input_desc = Input(&input_desc_buffer, "Enter task description...");

    auto modal_container = Container::Vertical({
        input_title,
        input_desc
    });

    auto screen = ScreenInteractive::TerminalOutput();

    auto renderer = Renderer(modal_container, [&] {
        
        // Approach 1 Execution Path: The modal sequence is initiated.
        if (is_modal_active) {
            // The dialogue header title is dynamically changed based on the operational context.
            std::string modal_title = is_edit_mode ? " EDIT ACTIVE TASK " : " CREATE NEW TASK ";
            
            return vbox({
                text("") | flex,
                vbox({
                    text(modal_title) | bold | color(Color::Cyan) | hcenter,
                    separator(),
                    text(""),
                    
                    hbox(text("  Title:       ") | bold, input_title->Render() | focus),
                    text(""),
                    hbox(text("  Description: ") | bold, input_desc->Render()),
                    text(""),
                    
                    separator(),
                    text(" [Enter] Confirm Allocation  |  [Esc] Abort and Close ") | center | dim
                }) 
                | border 
                | center 
                | size(WIDTH, EQUAL, 70) 
                | size(HEIGHT, EQUAL, 11),
                text("") | flex
            }) | center;
        }

        // Standard Execution Path: The main interactive dashboard grid is evaluated.
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        auto in_progress_tasks = board.getTasksByStatus(TaskStatus::IN_PROGRESS);
        auto done_tasks = board.getTasksByStatus(TaskStatus::DONE);

        auto render_column = [&](const std::string& title, const std::vector<::Task>& tasks, int column_id, Color header_color) {
            Elements task_elements;
            bool is_column_focused = (selected_column == column_id);

            if (is_column_focused && tasks.empty()) {
                task_elements.push_back(text("-> " + title + " <-") | bold | color(Color::Cyan) | hcenter);
            } else {
                task_elements.push_back(text("  " + title + "  ") | bold | color(header_color) | hcenter);
            }
            task_elements.push_back(separator());

            if (tasks.empty()) {
                auto placeholder_box = vbox({
                    text("No tasks available") | center,
                    text("Press [N] to create a new task") | center | dim
                });

                if (is_column_focused) {
                    task_elements.push_back(window(text("Empty Column") | color(Color::Cyan), placeholder_box) | color(Color::Cyan));
                } else {
                    task_elements.push_back(window(text("Empty"), placeholder_box) | dim);
                }
            } else {
                for (size_t i = 0; i < tasks.size(); ++i) {
                    bool is_task_focused = (is_column_focused && selected_task_index == static_cast<int>(i));
                    
                    auto task_box = vbox({
                        text(tasks[i].title) | bold,
                        text("ID: " + std::to_string(tasks[i].id) + " | " + tasks[i].created_at) | dim,
                        separatorDashed(),
                        text(tasks[i].description)
                    });

                    if (is_task_focused) {
                        task_elements.push_back(window(text("-> ACTIVE <-") | color(Color::Cyan), task_box) | color(Color::Cyan));
                    } else {
                        task_elements.push_back(window(text("Task"), task_box));
                    }
                }
            }

            return vbox(std::move(task_elements)) | flex;
        };

        auto board_layout = ftxui::hbox({
            render_column("TO DO", todo_tasks, 0, Color::Red),
            separator(),
            render_column("IN PROGRESS", in_progress_tasks, 1, Color::Yellow),
            separator(),
            render_column("DONE", done_tasks, 2, Color::Green),
        }) | flex;

        // Expanded helper legend layout includes the modification command shortcut.
        auto status_bar = ftxui::hbox({
            text(" [Arrows] Move Focus ") | bgcolor(Color::Blue) | bold,
            text(" [Space/Enter] Move Column ") | bgcolor(Color::Green),
            text(" [N] New ") | bgcolor(Color::Cyan),
            text(" [E] Edit ") | bgcolor(Color::Magenta), // Added visual shortcut for editing
            text(" [D] Delete ") | bgcolor(Color::GrayDark),
            text(" ") | flex,
            text(" [Q] Quit ") | bgcolor(Color::Red) | bold
        });

        return vbox({ board_layout, separator(), status_bar });
    });

    auto catch_event = CatchEvent(renderer, [&](Event event) {
        auto get_current_tasks_vector = [&]() -> std::vector<::Task> {
            if (selected_column == 0) return board.getTasksByStatus(TaskStatus::TODO);
            if (selected_column == 1) return board.getTasksByStatus(TaskStatus::IN_PROGRESS);
            return board.getTasksByStatus(TaskStatus::DONE);
        };

        if (is_modal_active) {
            if (event == Event::Escape) {
                // Input buffers and states are safely flushed upon aborting the context.
                input_title_buffer.clear();
                input_desc_buffer.clear();
                is_modal_active = false;
                is_edit_mode = false;
                editing_task_id = -1;
                return true;
            }
            if (event == Event::Return) {
                if (!input_title_buffer.empty()) {
                    // Operational routing is executed based on the active structural mode flag.
                    if (is_edit_mode) {
                        board.updateTaskDetails(editing_task_id, input_title_buffer, input_desc_buffer);
                    } else {
                        board.addTask(input_title_buffer, input_desc_buffer, TaskPriority::MEDIUM);
                    }
                    saveBoardState(board, storage);
                    
                    // Buffers and operational states are cleared post-commit.
                    input_title_buffer.clear();
                    input_desc_buffer.clear();
                    is_modal_active = false;
                    is_edit_mode = false;
                    editing_task_id = -1;
                }
                return true;
            }
            return modal_container->OnEvent(event);
        }

        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }

        if (event == Event::Character('n') || event == Event::Character('N')) {
            is_modal_active = true;
            is_edit_mode = false; // Ensures standard record entry behavior
            return true;
        }

        auto active_vector = get_current_tasks_vector();
        bool has_active_task = (!active_vector.empty() && selected_task_index < static_cast<int>(active_vector.size()));

        // Edit Command Execution Hook.
        if ((event == Event::Character('e') || event == Event::Character('E')) && has_active_task) {
            auto current_task = active_vector[selected_task_index];
            
            // UI buffers are pre-loaded with historical records to support localized text mutation.
            input_title_buffer = current_task.title;
            input_desc_buffer = current_task.description;
            
            // Operational flags are configured to activate the modification dialog context.
            editing_task_id = current_task.id;
            is_edit_mode = true;
            is_modal_active = true;
            return true;
        }

        if ((event == Event::Character('d') || event == Event::Character('D')) && has_active_task) {
            board.removeTask(active_vector[selected_task_index].id);
            saveBoardState(board, storage);
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (selected_task_index >= revised_size && revised_size > 0) {
                selected_task_index = revised_size - 1;
            }
            return true;
        }

        if ((event == Event::Special(" ") || event == Event::Return) && has_active_task) {
            auto task_to_move = active_vector[selected_task_index];
            if (selected_column == 0) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::IN_PROGRESS);
            } else if (selected_column == 1) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::DONE);
            }
            saveBoardState(board, storage);
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (selected_task_index >= revised_size && revised_size > 0) {
                selected_task_index = revised_size - 1;
            }
            return true;
        }

        if (event == Event::ArrowLeft) {
            selected_column = std::max(0, selected_column - 1);
            selected_task_index = 0;
            return true;
        }
        if (event == Event::ArrowRight) {
            selected_column = std::min(2, selected_column + 1);
            selected_task_index = 0;
            return true;
        }
        if (event == Event::ArrowUp) {
            selected_task_index = std::max(0, selected_task_index - 1);
            return true;
        }
        if (event == Event::ArrowDown) {
            int current_size = static_cast<int>(active_vector.size());
            if (current_size > 0) {
                selected_task_index = std::min(current_size - 1, selected_task_index + 1);
            }
            return true;
        }

        return false;
    });

    screen.Loop(catch_event);

    return 0;
}
