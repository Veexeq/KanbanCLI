#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <nlohmann/json.hpp>

int main() {
    using namespace ftxui;
    
    Element document = vbox({
        text("--- KANBAN BOARD ---") | bold | color(Color::Green),
        separator(),
        text("Srodowisko i biblioteki skonfigurowane pomyslnie!"),
        text("Standard: C++26 aktywowany."),
    }) | border;

    auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
    Render(screen, document);
    screen.Print();

    return 0;
}
