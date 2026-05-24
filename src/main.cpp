/**
 * @file main.cpp
 * @brief High-fidelity, responsive TUI Kanban board execution driver.
 */

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/screen/terminal.hpp> 
#include <vector>
#include <string>
#include <algorithm>
#include <iostream> 
#include "KanbanBoard.hpp"
#include "StorageManager.hpp"
#include "Theme.hpp"
#include "AppState.hpp"

void saveBoardState(const KanbanBoard& board, const StorageManager& storage) {
    std::vector<::Task> all_tasks;
    for (auto status : {TaskStatus::TODO, TaskStatus::IN_PROGRESS, TaskStatus::DONE}) {
        auto column_tasks = board.getTasksByStatus(status);
        all_tasks.insert(all_tasks.end(), column_tasks.begin(), column_tasks.end());
    }
    storage.saveTasks(all_tasks);
}

int main() {
    using namespace ftxui;

    StorageManager storage("kanban.json");
    KanbanBoard board;
    AppState state;

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
        saveBoardState(board, storage);
    }

    // Feature 2: Priority options array declaration and Toggle component instantiation
    std::vector<std::string> priority_options = { "  LOW  ", "  MEDIUM  ", "  HIGH  " };
    
    Component input_title = Input(&state.input_title_buffer, "Enter task title...");
    Component input_desc = Input(&state.input_desc_buffer, "Type descriptive details here...");
    Component input_priority = Toggle(&priority_options, &state.input_priority_index);

    // Grouping components inside the interactive modal layout container tree
    auto modal_container = Container::Vertical({
        input_title,
        input_desc,
        input_priority
    });

    auto screen = ScreenInteractive::TerminalOutput();

    auto renderer = Renderer(modal_container, [&] {
        auto terminal_dimensions = ftxui::Terminal::Size();
        
        int computed_column_width = (terminal_dimensions.dimx - Theme::HORIZONTAL_PADDING) / Theme::COLUMN_COUNT;
        if (computed_column_width < Theme::MIN_COLUMN_WIDTH) computed_column_width = Theme::MIN_COLUMN_WIDTH; 
        
        bool is_narrow_viewport = (terminal_dimensions.dimx < 115);
        int computed_board_height = terminal_dimensions.dimy - (is_narrow_viewport ? 3 : 2);
        if (computed_board_height < Theme::MIN_BOARD_HEIGHT) computed_board_height = Theme::MIN_BOARD_HEIGHT;

        int inside_card_wrap_limit = computed_column_width - 4;
        if (inside_card_wrap_limit < 5) inside_card_wrap_limit = 5;

        auto split_text_to_lines = [](const std::string& text_payload, size_t max_line_width) {
            Elements line_nodes;
            if (text_payload.empty()) {
                line_nodes.push_back(text("No content provided...") | dim);
                return line_nodes;
            }
            std::string current_line = "";
            std::string current_word = "";
            
            auto append_word = [&](const std::string& word) {
                std::string processed_chunk = word;
                while (processed_chunk.length() > max_line_width) {
                    if (!current_line.empty()) {
                        line_nodes.push_back(text(current_line));
                        current_line = "";
                    }
                    line_nodes.push_back(text(processed_chunk.substr(0, max_line_width)));
                    processed_chunk = processed_chunk.substr(max_line_width);
                }
                if (current_line.empty()) {
                    current_line = processed_chunk;
                } else if (current_line.length() + 1 + processed_chunk.length() > max_line_width) {
                    line_nodes.push_back(text(current_line));
                    current_line = processed_chunk;
                } else {
                    current_line += " " + processed_chunk;
                }
            };

            for (char character : text_payload) {
                if (character == ' ') {
                    if (!current_word.empty()) {
                        append_word(current_word);
                        current_word = "";
                    }
                } else {
                    current_word += character;
                }
            }
            if (!current_word.empty()) append_word(current_word);
            if (!current_line.empty()) line_nodes.push_back(text(current_line));
            return line_nodes;
        };

        // UI Execution Path A: Task Creation / Modification Dialog Layout Matrix
        if (state.is_modal_active) {
            std::string modal_title = state.is_edit_mode ? " EDIT ACTIVE TASK " : " CREATE NEW TASK ";
            return vbox({
                text("") | flex,
                vbox({
                    text(modal_title) | bold | color(Theme::COLOR_MODAL_TITLE) | hcenter,
                    separator(),
                    text(""),
                    hbox(text("  Title:       ") | bold, input_title->Render()),
                    text(""),
                    hbox(text("  Description: ") | bold, input_desc->Render()),
                    text(""),
                    // Feature 2 UI Placement: Rendering the horizontal priority selector
                    hbox(text("  Priority:    ") | bold, input_priority->Render() | color(Theme::COLOR_FOCUS)),
                    text(""),
                    window(text(" Description Canvas (Dynamic Multiline View) ") | color(Theme::COLOR_CANVAS_BORDER),
                           vbox(split_text_to_lines(state.input_desc_buffer, 65))
                    ),
                    text(""),
                    separator(),
                    text(" [Enter] Confirm Allocation  |  [Esc] Abort and Close ") | center | dim
                }) 
                | border 
                | center 
                | size(WIDTH, EQUAL, Theme::MODAL_WIDTH),
                text("") | flex
            }) | center;
        }

        // UI Execution Path B: UX Feature 3 Update - Confirmation Overlay View (Enter/Esc Matrix)
        if (state.is_confirming_delete) {
            return vbox({
                text("") | flex,
                vbox({
                    text(" !!! DELETION WARNING !!! ") | bold | color(ftxui::Color::Red) | hcenter,
                    separator(),
                    text(""),
                    text(" Are you sure you want to permanently delete this task? ") | hcenter,
                    text(" This action is destructive and cannot be undone. ") | hcenter | dim,
                    text(""),
                    separator(),
                    // Feature 3 Text Update: Inform user about Enter / Esc confirmation mechanics
                    text(" [Enter] Yes, Delete Record  |  [Esc] No, Cancel Operation ") | center | bold | color(ftxui::Color::Red)
                })
                | border 
                | center 
                | size(WIDTH, EQUAL, 65)
                | size(HEIGHT, EQUAL, 11),
                text("") | flex
            }) | center;
        }

        // UI Execution Path C: Standard Dashboard Presentation Canvas Layout
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        auto in_progress_tasks = board.getTasksByStatus(TaskStatus::IN_PROGRESS);
        auto done_tasks = board.getTasksByStatus(TaskStatus::DONE);

        auto render_column = [&](const std::string& title, const std::vector<::Task>& tasks, int column_id, Color header_color) {
            Elements header_elements;
            Elements content_elements;
            bool is_column_focused = (state.selected_column == column_id);

            if (is_column_focused && tasks.empty()) {
                header_elements.push_back(text("-> " + title + " <-") | bold | color(Theme::COLOR_FOCUS) | hcenter);
            } else {
                header_elements.push_back(text("  " + title + "  ") | bold | color(header_color) | hcenter);
            }
            header_elements.push_back(separator());

            if (tasks.empty()) {
                auto placeholder_box = vbox(split_text_to_lines("No tasks available. Press [N] to create.", inside_card_wrap_limit));
                if (is_column_focused) {
                    content_elements.push_back(window(text("Empty") | color(Theme::COLOR_FOCUS), placeholder_box) | color(Theme::COLOR_FOCUS));
                } else {
                    content_elements.push_back(window(text("Empty"), placeholder_box) | dim);
                }
            } else {
                for (size_t i = 0; i < tasks.size(); ++i) {
                    bool is_task_focused = (is_column_focused && state.selected_task_index == static_cast<int>(i));
                    
                    Color priority_tag_color = Theme::COLOR_PRIORITY_MEDIUM;
                    std::string priority_label = "MED";
                    if (tasks[i].priority == TaskPriority::HIGH) {
                        priority_tag_color = Theme::COLOR_PRIORITY_HIGH;
                        priority_label = "HIGH";
                    } else if (tasks[i].priority == TaskPriority::LOW) {
                        priority_tag_color = Theme::COLOR_PRIORITY_LOW;
                        priority_label = "LOW";
                    }

                    auto task_box = vbox({
                        hbox({
                            vbox(split_text_to_lines(tasks[i].title, inside_card_wrap_limit - 8)) | flex | bold,
                            text("[" + priority_label + "]") | color(priority_tag_color) | bold
                        }),
                        text("ID: " + std::to_string(tasks[i].id) + " | " + tasks[i].created_at.substr(0, 10)) | dim,
                        separatorDashed(),
                        vbox(split_text_to_lines(tasks[i].description, inside_card_wrap_limit)) 
                    });

                    if (is_task_focused) {
                        content_elements.push_back(window(text("Active") | color(Theme::COLOR_FOCUS), task_box) | color(Theme::COLOR_FOCUS) | focus);
                    } else {
                        content_elements.push_back(window(text("Task"), task_box) | color(priority_tag_color));
                    }
                }
            }

            return vbox({
                vbox(std::move(header_elements)),
                vbox(std::move(content_elements)) | frame | vscroll_indicator | flex
            }) 
            | size(WIDTH, EQUAL, computed_column_width)
            | flex; 
        };

        auto board_layout = ftxui::hbox({
            render_column("TO DO", todo_tasks, 0, Theme::COLOR_TODO_HEADER),
            separator(),
            render_column("IN PROGRESS", in_progress_tasks, 1, Theme::COLOR_PROGRESS_HEADER),
            separator(),
            render_column("DONE", done_tasks, 2, Theme::COLOR_DONE_HEADER),
        }) | size(HEIGHT, EQUAL, computed_board_height); 

        Element status_bar;
        if (is_narrow_viewport) {
            status_bar = vbox({
                hbox({
                    text(" [Arrows] Focus Grid ") | bgcolor(Color::Blue) | bold,
                    text(" [Space] Move Forward ") | bgcolor(Color::Green),
                    text(" [Backspace] Move Back ") | bgcolor(Color::Yellow) | color(Color::Black)
                }),
                hbox({
                    text(" [N] New ") | bgcolor(Color::Cyan),
                    text(" [E] Edit ") | bgcolor(Color::Magenta), 
                    text(" [D] Delete ") | bgcolor(Color::GrayDark),
                    text(" ") | flex,
                    text(" [Q] Quit ") | bgcolor(Color::Red) | bold
                })
            });
        } else {
            status_bar = ftxui::hbox({
                text(" [Arrows] Move Focus ") | bgcolor(Color::Blue) | bold,
                text(" [Space/Enter] Forward ") | bgcolor(Color::Green),
                text(" [Backspace] Back ") | bgcolor(Color::Yellow) | color(Color::Black),
                text(" [N] New ") | bgcolor(Color::Cyan),
                text(" [E] Edit ") | bgcolor(Color::Magenta), 
                text(" [D] Delete ") | bgcolor(Color::GrayDark),
                text(" ") | flex,
                text(" [Q] Quit ") | bgcolor(Color::Red) | bold
            });
        }

        return vbox({ board_layout, separator(), status_bar });
    });

    auto catch_event = CatchEvent(renderer, [&](Event event) {
        auto get_current_tasks_vector = [&]() -> std::vector<::Task> {
            if (state.selected_column == 0) return board.getTasksByStatus(TaskStatus::TODO);
            if (state.selected_column == 1) return board.getTasksByStatus(TaskStatus::IN_PROGRESS);
            return board.getTasksByStatus(TaskStatus::DONE);
        };

        // Feature 3 Logic Check: Overriding deletion warning confirmations to handle Enter / Escape
        if (state.is_confirming_delete) {
            if (event == Event::Return) { // Enter confirms deletion
                auto active_vector = get_current_tasks_vector();
                if (!active_vector.empty() && state.selected_task_index < static_cast<int>(active_vector.size())) {
                    board.removeTask(active_vector[state.selected_task_index].id);
                    saveBoardState(board, storage);
                    
                    int revised_size = static_cast<int>(get_current_tasks_vector().size());
                    if (state.selected_task_index >= revised_size && revised_size > 0) {
                        state.selected_task_index = revised_size - 1;
                    }
                }
                state.flushBuffers();
                return true;
            }
            if (event == Event::Escape) { // Escape aborts deletion cleanly
                state.flushBuffers();
                return true;
            }
            return true; 
        }

        if (state.is_modal_active) {
            if (event == Event::Escape) {
                state.flushBuffers();
                return true;
            }
            if (event == Event::Return) {
                if (!state.input_title_buffer.empty()) {
                    // Feature 2: Evaluation of the selected priority enum mapping
                    TaskPriority p = TaskPriority::MEDIUM;
                    if (state.input_priority_index == 0) p = TaskPriority::LOW;
                    else if (state.input_priority_index == 2) p = TaskPriority::HIGH;

                    if (state.is_edit_mode) {
                        board.updateTaskDetails(state.editing_task_id, state.input_title_buffer, state.input_desc_buffer, p);
                    } else {
                        board.addTask(state.input_title_buffer, state.input_desc_buffer, p);
                    }
                    saveBoardState(board, storage);
                    state.flushBuffers();
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
            state.is_modal_active = true;
            state.is_edit_mode = false; 
            return true;
        }

        auto active_vector = get_current_tasks_vector();
        bool has_active_task = (!active_vector.empty() && state.selected_task_index < static_cast<int>(active_vector.size()));

        if ((event == Event::Character('d') || event == Event::Character('D')) && has_active_task) {
            state.is_confirming_delete = true;
            return true;
        }

        if ((event == Event::Character('e') || event == Event::Character('E')) && has_active_task) {
            auto current_task = active_vector[state.selected_task_index];
            state.input_title_buffer = current_task.title;
            state.input_desc_buffer = current_task.description;
            state.editing_task_id = current_task.id;
            
            // Map the active enum back to the UI index buffer when opening edit mode
            if (current_task.priority == TaskPriority::LOW) state.input_priority_index = 0;
            else if (current_task.priority == TaskPriority::HIGH) state.input_priority_index = 2;
            else state.input_priority_index = 1;

            state.is_edit_mode = true;
            state.is_modal_active = true;
            return true;
        }

        if ((event == Event::Special(" ") || event == Event::Return) && has_active_task) {
            auto task_to_move = active_vector[state.selected_task_index];
            if (state.selected_column == 0) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::IN_PROGRESS);
            } else if (state.selected_column == 1) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::DONE);
            }
            saveBoardState(board, storage);
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (state.selected_task_index >= revised_size && revised_size > 0) {
                state.selected_task_index = revised_size - 1;
            }
            return true;
        }

        if (event == Event::Backspace && has_active_task) {
            auto task_to_move = active_vector[state.selected_task_index];
            if (state.selected_column == 2) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::IN_PROGRESS);
            } else if (state.selected_column == 1) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::TODO);
            }
            saveBoardState(board, storage);
            int revised_size = static_cast<int>(get_current_tasks_vector().size());
            if (state.selected_task_index >= revised_size && revised_size > 0) {
                state.selected_task_index = revised_size - 1;
            }
            return true;
        }

        if (event == Event::ArrowLeft) {
            state.selected_column = std::max(0, state.selected_column - 1);
            state.selected_task_index = 0;
            return true;
        }
        if (event == Event::ArrowRight) {
            state.selected_column = std::min(2, state.selected_column + 1);
            state.selected_task_index = 0;
            return true;
        }
        if (event == Event::ArrowUp) {
            state.selected_task_index = std::max(0, state.selected_task_index - 1);
            return true;
        }
        if (event == Event::ArrowDown) {
            int current_size = static_cast<int>(active_vector.size());
            if (current_size > 0) {
                state.selected_task_index = std::min(current_size - 1, state.selected_task_index + 1);
            }
            return true;
        }

        return false;
    });

    std::cout << "\033[2J\033[H" << std::flush;
    screen.Loop(catch_event);

    return 0;
}
