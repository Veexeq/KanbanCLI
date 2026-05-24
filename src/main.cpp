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
    bool is_edit_mode = false;      
    int editing_task_id = -1;       
    
    std::string input_title_buffer;
    std::string input_desc_buffer;

    Component input_title = Input(&input_title_buffer, "Enter task title...");
    Component input_desc = Input(&input_desc_buffer, "Type descriptive details here...");

    auto modal_container = Container::Vertical({
        input_title,
        input_desc
    });

    auto screen = ScreenInteractive::TerminalOutput();

    auto renderer = Renderer(modal_container, [&] {
        
        // Dynamic structural metrics are fetched directly from the active console buffer.
        auto terminal_dimensions = ftxui::Terminal::Size();
        
        // Strict geometric width allocations are calculated to guarantee a perfect 33% grid split.
        int computed_column_width = (terminal_dimensions.dimx - 2) / 3;
        if (computed_column_width < 15) computed_column_width = 15; 
        
        // Responsive layout toggle thresholds are checked to dynamically re-scale heights.
        bool is_narrow_viewport = (terminal_dimensions.dimx < 115); // Width extended to accommodate extra hotkey legend
        
        int computed_board_height = terminal_dimensions.dimy - (is_narrow_viewport ? 3 : 2);
        if (computed_board_height < 5) computed_board_height = 5;

        // Safe line wrapping limits are established based on the active dynamic column bounds.
        int inside_card_wrap_limit = computed_column_width - 4;
        if (inside_card_wrap_limit < 5) inside_card_wrap_limit = 5;

        // A robust custom text tokenizer lambda is established to slice strings cleanly.
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
            if (!current_word.empty()) {
                append_word(current_word);
            }
            if (!current_line.empty()) {
                line_nodes.push_back(text(current_line));
            }
            return line_nodes;
        };

        // Approach 1 Execution Path: The modal sequence is initiated.
        if (is_modal_active) {
            std::string modal_title = is_edit_mode ? " EDIT ACTIVE TASK " : " CREATE NEW TASK ";
            
            return vbox({
                text("") | flex,
                vbox({
                    text(modal_title) | bold | color(Color::Cyan) | hcenter,
                    separator(),
                    text(""),
                    
                    hbox(text("  Title:       ") | bold, input_title->Render()),
                    text(""),
                    hbox(text("  Input Line:  ") | bold, input_desc->Render()),
                    text(""),
                    
                    window(text(" Description Canvas (Dynamic Multiline View) ") | color(Color::Yellow),
                           vbox(split_text_to_lines(input_desc_buffer, 65))
                    ),
                    text(""),
                    
                    separator(),
                    text(" [Enter] Confirm Allocation  |  [Esc] Abort and Close ") | center | dim
                }) 
                | border 
                | center 
                | size(WIDTH, EQUAL, 75),
                text("") | flex
            }) | center;
        }

        // Standard Execution Path: The main interactive dashboard grid is evaluated.
        auto todo_tasks = board.getTasksByStatus(TaskStatus::TODO);
        auto in_progress_tasks = board.getTasksByStatus(TaskStatus::IN_PROGRESS);
        auto done_tasks = board.getTasksByStatus(TaskStatus::DONE);

        auto render_column = [&](const std::string& title, const std::vector<::Task>& tasks, int column_id, Color header_color) {
            Elements header_elements;
            Elements content_elements;
            bool is_column_focused = (selected_column == column_id);

            if (is_column_focused && tasks.empty()) {
                header_elements.push_back(text("-> " + title + " <-") | bold | color(Color::Cyan) | hcenter);
            } else {
                header_elements.push_back(text("  " + title + "  ") | bold | color(header_color) | hcenter);
            }
            header_elements.push_back(separator());

            if (tasks.empty()) {
                auto placeholder_box = vbox(split_text_to_lines("No tasks available. Press [N] to create.", inside_card_wrap_limit));

                if (is_column_focused) {
                    content_elements.push_back(window(text("Empty") | color(Color::Cyan), placeholder_box) | color(Color::Cyan));
                } else {
                    content_elements.push_back(window(text("Empty"), placeholder_box) | dim);
                }
            } else {
                for (size_t i = 0; i < tasks.size(); ++i) {
                    bool is_task_focused = (is_column_focused && selected_task_index == static_cast<int>(i));
                    
                    auto task_box = vbox({
                        vbox(split_text_to_lines(tasks[i].title, inside_card_wrap_limit)) | bold,
                        text("ID: " + std::to_string(tasks[i].id) + " | " + tasks[i].created_at.substr(0, 10)) | dim,
                        separatorDashed(),
                        vbox(split_text_to_lines(tasks[i].description, inside_card_wrap_limit)) 
                    });

                    if (is_task_focused) {
                        content_elements.push_back(window(text("Active") | color(Color::Cyan), task_box) | color(Color::Cyan) | focus);
                    } else {
                        content_elements.push_back(window(text("Task"), task_box));
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

        // Separate columns are aligned horizontally.
        auto board_layout = ftxui::hbox({
            render_column("TO DO", todo_tasks, 0, Color::Red),
            separator(),
            render_column("IN PROGRESS", in_progress_tasks, 1, Color::Yellow),
            separator(),
            render_column("DONE", done_tasks, 2, Color::Green),
        }) | size(HEIGHT, EQUAL, computed_board_height); 

        // The interface control legend responds dynamically, now featuring the Backward command tracking.
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
            if (selected_column == 0) return board.getTasksByStatus(TaskStatus::TODO);
            if (selected_column == 1) return board.getTasksByStatus(TaskStatus::IN_PROGRESS);
            return board.getTasksByStatus(TaskStatus::DONE);
        };

        if (is_modal_active) {
            if (event == Event::Escape) {
                input_title_buffer.clear();
                input_desc_buffer.clear();
                is_modal_active = false;
                is_edit_mode = false;
                editing_task_id = -1;
                return true;
            }
            if (event == Event::Return) {
                if (!input_title_buffer.empty()) {
                    if (is_edit_mode) {
                        board.updateTaskDetails(editing_task_id, input_title_buffer, input_desc_buffer);
                    } else {
                        board.addTask(input_title_buffer, input_desc_buffer, TaskPriority::MEDIUM);
                    }
                    saveBoardState(board, storage);
                    
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
            is_edit_mode = false; 
            return true;
        }

        auto active_vector = get_current_tasks_vector();
        bool has_active_task = (!active_vector.empty() && selected_task_index < static_cast<int>(active_vector.size()));

        if ((event == Event::Character('e') || event == Event::Character('E')) && has_active_task) {
            auto current_task = active_vector[selected_task_index];
            input_title_buffer = current_task.title;
            input_desc_buffer = current_task.description;
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

        // Advanced Transition State Hook: Forward Direction (Space or Enter).
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

        // Advanced Transition State Hook: Backward Direction (Backspace).
        if (event == Event::Backspace && has_active_task) {
            auto task_to_move = active_vector[selected_task_index];
            if (selected_column == 2) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::IN_PROGRESS);
            } else if (selected_column == 1) {
                board.updateTaskStatus(task_to_move.id, TaskStatus::TODO);
            }
            saveBoardState(board, storage);
            
            // Recalculate focus vector bounds to handle structural array shifts safely.
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

    std::cout << "\033[2J\033[H" << std::flush;

    screen.Loop(catch_event);

    return 0;
}
