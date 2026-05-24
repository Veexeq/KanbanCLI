/**
 * @file Theme.hpp
 * @brief Contains global style configurations, dimensions, and color schemes for the Kanban board UI.
 */

#ifndef THEME_HPP
#define THEME_HPP

#include <ftxui/screen/color.hpp>

/**
 * @namespace Theme
 * @brief Provides centralized access to semantic design constants.
 */
namespace Theme {
    // Structural geometry configuration parameters
    constexpr int MIN_COLUMN_WIDTH = 15;      ///< Absolute lower ceiling width constraint for any dashboard column.
    constexpr int MIN_BOARD_HEIGHT = 5;       ///< Lower safety floor height limitation for the main grid.
    constexpr int HORIZONTAL_PADDING = 2;     ///< Total columns reserved for structural grid spacing separators.
    constexpr int COLUMN_COUNT = 3;           ///< The invariant total count of functional workflow swimlanes.
    
    // Modal dialogue layout metrics
    constexpr int MODAL_WIDTH = 75;           ///< Hard geometric width constraint for the standalone task data window.

    // Core application semantic color assignments
    const ftxui::Color COLOR_FOCUS = ftxui::Color::Cyan;     ///< Frame highlight color reserved for the currently active component.
    const ftxui::Color COLOR_MODAL_TITLE = ftxui::Color::Cyan;///< Color mapping for the creation/modification layout titles.
    const ftxui::Color COLOR_CANVAS_BORDER = ftxui::Color::Yellow; ///< Frame decoration tint applied to the dynamic description preview area.

    // Primary workflow swimlane header tint matrices
    const ftxui::Color COLOR_TODO_HEADER = ftxui::Color::Red;       ///< Header color indicator for the backlog column block.
    const ftxui::Color COLOR_PROGRESS_HEADER = ftxui::Color::Yellow; ///< Header color indicator for the active development column block.
    const ftxui::Color COLOR_DONE_HEADER = ftxui::Color::Green;     ///< Header color indicator for the completed work column block.

    // UX Feature 4: Priority decoration color metrics mapping matrix
    const ftxui::Color COLOR_PRIORITY_HIGH = ftxui::Color::RedLight;    ///< Visual accent color assigned to high urgency items.
    const ftxui::Color COLOR_PRIORITY_MEDIUM = ftxui::Color::YellowLight; ///< Visual accent color assigned to standard urgency items.
    const ftxui::Color COLOR_PRIORITY_LOW = ftxui::Color::GreenLight;   ///< Visual accent color assigned to routine maintenance elements.
}

#endif // THEME_HPP
