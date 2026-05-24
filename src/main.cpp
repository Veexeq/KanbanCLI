#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include "KanbanBoard.hpp"

int main() {
    using namespace ftxui;

    // The data domain container is initialized.
    KanbanBoard board;
    
    // Initial sample tasks are appended to populate the board state.
    board.addTask("Write presentation", "Prepare slides for university demo", TaskPriority::HIGH);
    board.addTask("Refactor code", "Clean up variable names and enforce consistency", TaskPriority::MEDIUM);
    
    // One task is transitioned to another column for initial layout demonstration.
    board.updateTaskStatus(2, TaskStatus::IN_PROGRESS);

    // Coordinate state trackers for the navigation grid are declared.
    int selected_column = 0;       // 0: TODO, 1: IN_PROGRESS, 2: DONE
    int selected_task_index = 0;   // Vertical index within the currently focused column

    // The interactive terminal screen display manager is instantiated.
    auto screen = ScreenInteractive::TerminalOutput();

    // The declarative layout UI structure is defined within the renderer component.
    auto renderer = Renderer([&] {
        // Task collections are fetched dynamically on each frame refresh.
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        auto in_progress_tasks = board.getTasksByStatus(TaskStatus::IN_PROGRESS);
        auto done_tasks = board.getTasksByStatus(TaskStatus::DONE);

        // A reusable layout generator for specific columns is established.
        auto render_column = [&](const std::string& title, const std::vector<::Task>& tasks, int column_id, Color header_color) {
            Elements task_elements;
            bool is_column_focused = (selected_column == column_id);

            // Empty column - solution 1: The header text and styling are altered dynamically if an empty column is focused.
            if (is_column_focused && tasks.empty()) {
                task_elements.push_back(text("-> " + title + " <-") | bold | color(Color::Cyan) | hcenter);
            } else {
                task_elements.push_back(text("  " + title + "  ") | bold | color(header_color) | hcenter);
            }
            task_elements.push_back(separator());

            // Empty column - solution 2: A dedicated placeholder is generated and rendered when the task vector is empty.
            if (tasks.empty()) {
                auto placeholder_box = vbox({
                    text("No tasks available") | center,
                    text("Press [N] to create a new task") | center | dim
                });

                // The placeholder component layout is styled based on the column focus state.
                if (is_column_focused) {
                    task_elements.push_back(window(text("Empty Column") | color(Color::Cyan), placeholder_box) | color(Color::Cyan));
                } else {
                    task_elements.push_back(window(text("Empty"), placeholder_box) | dim);
                }
            } else {
                // Individual task elements are iterated and structurally transformed into visual nodes.
                for (size_t i = 0; i < tasks.size(); ++i) {
                    bool is_task_focused = (is_column_focused && selected_task_index == static_cast<int>(i));
                    
                    auto task_box = vbox({
                        text(tasks[i].title) | bold,
                        text("ID: " + std::to_string(tasks[i].id) + " | " + tasks[i].created_at) | dim,
                        separatorDashed(),
                        text(tasks[i].description)
                    });

                    // Distinct borders are applied based on the current focus state coordinate matrix.
                    if (is_task_focused) {
                        task_elements.push_back(window(text("-> ACTIVE <-") | color(Color::Cyan), task_box) | color(Color::Cyan));
                    } else {
                        task_elements.push_back(window(text("Task"), task_box));
                    }
                }
            }

            return vbox(std::move(task_elements)) | flex;
        };

        // Separate columns are aligned horizontally across the screen matrix.
        auto board_layout = ftxui::hbox({
            render_column("TO DO", todo_tasks, 0, Color::Red),
            separator(),
            render_column("IN PROGRESS", in_progress_tasks, 1, Color::Yellow),
            separator(),
            render_column("DONE", done_tasks, 2, Color::Green),
        }) | flex;

        // The functional interface action legend is structured at the bottom.
        auto status_bar = ftxui::hbox({
            text(" [Arrows] Navigate ") | bgcolor(Color::Blue) | bold,
            text(" [Q] Quit ") | bgcolor(Color::Red) | bold
        });

        return vbox({ board_layout, separator(), status_bar });
    });

    // Keyboard events are intercepted and processed within the runtime block.
    auto catch_event = CatchEvent(renderer, [&](Event event) {
        // Task vectors are re-evaluated to determine runtime boundaries during keyboard updates.
        auto get_current_column_size = [&]() -> int {
            if (selected_column == 0) return static_cast<int>(board.getTasksByStatus(TaskStatus::TODO).size());
            if (selected_column == 1) return static_cast<int>(board.getTasksByStatus(TaskStatus::IN_PROGRESS).size());
            return static_cast<int>(board.getTasksByStatus(TaskStatus::DONE).size());
        };

        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }

        // Horizontal navigation boundaries are evaluated and applied.
        if (event == Event::ArrowLeft) {
            selected_column = std::max(0, selected_column - 1);
            selected_task_index = 0; // The vertical index is reset on column change to prevent out-of-bounds exceptions.
            return true;
        }
        if (event == Event::ArrowRight) {
            selected_column = std::min(2, selected_column + 1);
            selected_task_index = 0; // The vertical index is reset on column change to prevent out-of-bounds exceptions.
            return true;
        }

        // Vertical navigation boundaries are evaluated against the current task vector size.
        int current_size = get_current_column_size();
        if (event == Event::ArrowUp) {
            selected_task_index = std::max(0, selected_task_index - 1);
            return true;
        }
        if (event == Event::ArrowDown) {
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
