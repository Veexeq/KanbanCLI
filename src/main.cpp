#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>

int main() {
    using namespace ftxui;

    // An interactive screen that takes control over the terminal window
    auto screen = ScreenInteractive::TerminalOutput();

    auto renderer = Renderer([&] {
        
        auto todo_column = vbox({
            text("  TO DO  ") | bold | color(Color::Red) | hcenter,
            separator(),
            window(text("Task #1"), text("Fix compilation errors\nPriority: HIGH")),
            window(text("Task #2"), text("Setup Catch2 testing\nPriority: MEDIUM")),
        }) | flex;

        auto in_progress_column = vbox({
            text("  IN PROGRESS  ") | bold | color(Color::Yellow) | hcenter,
            separator(),
            window(text("Task #3"), text("Design TUI Layout\nPriority: HIGH")),
        }) | flex;

        auto done_column = vbox({
            text("  DONE  ") | bold | color(Color::Green) | hcenter,
            separator(),
            window(text("Task #0"), text("Core data structures\nPriority: LOW")) | dim,
        }) | flex;

        auto status_bar = hbox({
            text(" [N] New Task ") | bgcolor(Color::Blue) | bold,
            text(" [D] Delete Task ") | bgcolor(Color::GrayDark),

            // This empty flex pushes "quit" to the right edge
            text(" ") | flex,
            
            text(" [Q] Quit ") | bgcolor(Color::Red) | bold
        });

        return vbox({
            hbox({
                todo_column,
                separator(),
                in_progress_column,
                separator(),
                done_column
            }) | flex, // This flex makes the columns span the whole screen, except for the status bar
            separator(),
            status_bar
        });
    });

    auto catch_event = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(catch_event);

    return 0;
}
