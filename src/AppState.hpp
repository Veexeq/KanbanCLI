#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include <string>

/**
 * @struct AppState
 * @brief Centralized structural container isolating runtime UI focus indexes and input strings.
 */
struct AppState {
    std::string input_title_buffer = "";
    std::string input_desc_buffer = "";
    
    // Feature 2 Tracking: 0 = LOW, 1 = MEDIUM, 2 = HIGH
    int input_priority_index = 1; 

    bool is_modal_active = false;
    bool is_edit_mode = false;
    int editing_task_id = -1;

    int selected_column = 0;
    int selected_task_index = 0;
    bool is_confirming_delete = false;

    /**
     * @brief Resets all operational input string buffers and state flags to default production values.
     */
    void flushBuffers() {
        input_title_buffer = "";
        input_desc_buffer = "";
        input_priority_index = 1; // Default back to MEDIUM
        is_modal_active = false;
        is_edit_mode = false;
        editing_task_id = -1;
        is_confirming_delete = false;
    }
};

#endif // APP_STATE_HPP
