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
            // Tasks are re-registered into the active business logic model.
            board.addTask(task.title, task.description, task.priority);
            
            // The runtime status layout is updated to match the historical record.
            if (task.status != TaskStatus::TODO) {
                // The task is advanced to its correct column location.
                auto active_tasks = board.getTasksByStatus(TaskStatus::TODO);
                if (!active_tasks.empty()) {
                    board.updateTaskStatus(active_tasks.back().id, task.status);
                }
            }
        }
    } else {
        // Fallback demo elements are generated only if no database file is detected.
        board.addTask("Write presentation", "Prepare slides for university demo", TaskPriority::HIGH);
        board.addTask("Refactor code", "Clean up variable names and enforce consistency", TaskPriority::MEDIUM);
        board.addTask("Buy coffee", "Crucial resource for long programming sessions", TaskPriority::LOW);
        board.updateTaskStatus(2, TaskStatus::IN_PROGRESS);
        saveBoardState(board, storage);
    }

    // Coordinate state trackers for the grid matrix are declared.
    int selected_column = 0;       // 0: TODO, 1: IN_PROGRESS, 2: DONE
    int selected_task_index = 0;   // Vertical index within the currently focused column

    // Modal view tracking variables and input buffers are established.
    bool is_modal_active = false;
    std::string input_title_buffer;
    std::string input_desc_buffer;

    // Interactive FTXUI component elements are generated for input recording.
    Component input_title = Input(&input_title_buffer, "Enter task title...");
    Component input_desc = Input(&input_desc_buffer, "Enter task description...");

    // The sub-components are combined inside a structured vertical block container.
    auto modal_container = Container::Vertical({
        input_title,
        input_desc
    });

    // The interactive terminal screen display manager is instantiated.
    auto screen = ScreenInteractive::TerminalOutput();

    // The declarative layout UI structure is defined within the main renderer loop.
    auto renderer = Renderer(modal_container, [&] {
        // Active task collections are fetched dynamically on each layout loop.
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        auto in_progress_tasks = board.getTasksByStatus(TaskStatus::IN_PROGRESS);
        auto done_tasks = board.getTasksByStatus(TaskStatus::DONE);

        // A reusable layout builder for specific columns is established.
        auto render_column = [&](const std::string& title, const std::vector<::Task>& tasks, int column_id, Color header_color) {
            Elements task_elements;
            bool is_column_focused = (selected_column == column_id);

            // The header text is rendered with custom cyan focus tracking on empty blocks.
            if (is_column_focused && tasks.empty()) {
                task_elements.push_back(text("-> " + title + " <-") | bold | color(Color::Cyan) | hcenter);
            } else {
                task_elements.push_back(text("  " + title + "  ") | bold | color(header_color) | hcenter);
            }
            task_elements.push_back(separator());

            // A visual placeholder card is displayed if no entities are contained.
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
                // Task vectors are structurally transformed into styled interface nodes.
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

        // Columns are horizontally grouped to form the core dashboard matrix layout.
        auto board_layout = ftxui::hbox({
            render_column("TO DO", todo_tasks, 0, Color::Red),
            separator(),
            render_column("IN PROGRESS", in_progress_tasks, 1, Color::Yellow),
            separator(),
            render_column("DONE", done_tasks, 2, Color::Green),
        }) | flex;

        // The status navigation helper legend is formatted for the interface floor.
        auto status_bar = ftxui::hbox({
            text(" [Arrows] Move Focus ") | bgcolor(Color::Blue) | bold,
            text(" [Space/Enter] Transition Task ") | bgcolor(Color::Green),
            text(" [N] New Task ") | bgcolor(Color::Cyan),
            text(" [D] Delete ") | bgcolor(Color::GrayDark),
            text(" ") | flex,
            text(" [Q] Quit ") | bgcolor(Color::Red) | bold
        });

        // The primary dashboard view is aggregated inside a unified vertical canvas frame.
        auto main_dashboard_view = vbox({ board_layout, separator(), status_bar });

        // If the modal layer toggle is active, an overlay layout interface is rendered.
        if (is_modal_active) {
            auto modal_box = vbox({
                text(" CREATE NEW TASK ") | bold | color(Color::Cyan) | hcenter,
                separator(),
                hbox(text("Title:       ") | bold, input_title->Render() | focus),
                separatorDashed(),
                hbox(text("Description: ") | bold, input_desc->Render()),
                separator(),
                text(" [Enter] Confirm Allocation  |  [Esc] Abort and Close ") | center | dim
            }) | border | bgcolor(Color::Black) | center | size(WIDTH, LESS_THAN, 60);

            // The component layering is simplified by removing the undefined clear_panel helper.
            return dbox({
                main_dashboard_view,
                modal_box
            });
        }

        return main_dashboard_view;
    });

    // Keyboard capture sequences are parsed and intercepted within the global runtime event block.
    auto catch_event = CatchEvent(renderer, [&](Event event) {
        // Current collection capacities are monitored to enforce boundary limitations.
        auto get_current_tasks_vector = [&]() -> std::vector<::Task> {
            if (selected_column == 0) return board.getTasksByStatus(TaskStatus::TODO);
            if (selected_column == 1) return board.getTasksByStatus(TaskStatus::IN_PROGRESS);
            return board.getTasksByStatus(TaskStatus::DONE);
        };

        // Execution Path A: Keyboard tracking when the modal dialogue layer is active.
        if (is_modal_active) {
            if (event == Event::Escape) {
                is_modal_active = false;
                return true;
            }
            if (event == Event::Return) {
                if (!input_title_buffer.empty()) {
                    // The filled payload is appended to the business logic engine.
                    board.addTask(input_title_buffer, input_desc_buffer, TaskPriority::MEDIUM);
                    saveBoardState(board, storage);
                    
                    // Buffers are flushed and focus is returned to the dashboard.
                    input_title_buffer.clear();
                    input_desc_buffer.clear();
                    is_modal_active = false;
                }
                return true;
            }
            
            // Input event loops are directed towards active string capturing components.
            return modal_container->OnEvent(event);
        }

        // Execution Path B: Standard control mapping tracking across the dashboard view.
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }

        if (event == Event::Character('n') || event == Event::Character('N')) {
            is_modal_active = true;
            return true;
        }

        auto active_vector = get_current_tasks_vector();
        bool has_active_task = (!active_vector.empty() && selected_task_index < static_cast<int>(active_vector.size()));

        // Delete Command Execution Hook.
        if ((event == Event::Character('d') || event == Event::Character('D')) && has_active_task) {
            board.removeTask(active_vector[selected_task_index].id);
            saveBoardState(board, storage);
            
            // Focused index coordinates are recalculated to fit within the shrunken collection.
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (selected_task_index >= revised_size && revised_size > 0) {
                selected_task_index = revised_size - 1;
            }
            return true;
        }

        // Advanced Transition State Hook (Space or Enter).
        if ((event == Event::Special(" ") || event == Event::Return) && has_active_task) {
            auto task_to_move = active_vector[selected_task_index];
            if (selected_column == 0) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::IN_PROGRESS);
            } else if (selected_column == 1) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::DONE);
            }
            saveBoardState(board, storage);
            
            // Structural layout indexes are checked to prevent null reference highlights.
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (selected_task_index >= revised_size && revised_size > 0) {
                selected_task_index = revised_size - 1;
            }
            return true;
        }

        // Grid navigation controls processing.
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

    // The runtime event polling engine is activated.
    screen.Loop(catch_event);

    return 0;
}
