/**
 * @file AppState.hpp
 * @brief Manages the decoupled global interface states, tracking indexes, and text buffers.
 */

#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include <string>

/**
 * @struct AppState
 * @brief Encapsulates navigation coordinates, dialogue toggles, and data ingestion buffers.
 */
struct AppState {
    // 2D Matrix coordinate focus trackers
    int selected_column = 0;       ///< Operational horizontal index mapping (0: TODO, 1: IN_PROGRESS, 2: DONE).
    int selected_task_index = 0;   ///< Operational vertical selection pointer mapping inside the focused column list.

    // Control routing logical flow flags
    bool is_modal_active = false;      ///< Interception toggle routing execution paths to the data dialogue layer when true.
    bool is_edit_mode = false;         ///< Context configuration router driving the modal processing split (false: Create, true: Mutate).
    bool is_confirming_delete = false; ///< UX Feature 4: Safety gate intercepting immediate execution blocks to handle prompt verification.

    // Active record identifier cache tracking
    int editing_task_id = -1;      ///< Cache holder locking the domain model sequence identifier during modification updates.

    // User interaction input character data buffers
    std::string input_title_buffer; ///< Live keystroke capture sink bound to the task title creation field.
    std::string input_desc_buffer;  ///< Live keystroke capture sink bound to the task description creation field.

    /**
     * @brief Resets the structural interaction input string buffers and contextual operational states.
     */
    void flushBuffers() {
        input_title_buffer.clear();
        input_desc_buffer.clear();
        is_modal_active = false;
        is_edit_mode = false;
        is_confirming_delete = false;
        editing_task_id = -1;
    }
};

#endif // APP_STATE_HPP
