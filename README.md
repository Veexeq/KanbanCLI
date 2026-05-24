# 📊 Kanban Board TUI (Terminal User Interface)

A high-fidelity, highly responsive, and interactive Kanban board built natively in **C++20** using the **FTXUI** terminal layout engine. Designed as a professional tool for lightweight task organization directly within the desktop terminal shell environment.

---

## 🚀 Quick Start (For Users & Evaluators)

You do not need any programming environment, CMake, or compilers installed to run this application. A fully pre-compiled package is available for instant execution.

### 📦 1-Click Launch Setup

1. Head over to the **[Releases](https://github.com/Veexeq/KanbanCLI/releases/tag/v1.0.4)** section on the right side of this repository and download the latest `kanban_board_windows.zip`.
2. **CRITICAL SECURITY STEP:** Before unzipping, right-click on the downloaded `kanban_board_windows.zip` file, select **Properties**, check the **Unblock** (*Odblokuj*) checkbox at the bottom safety section, and click **Apply**. *(This removes the OS download flag to prevent unsigned security prompts).*
3. Extract the ZIP archive contents anywhere on your local storage drive.
4. Double-click the **`run_kanban.bat`** file script. 

The application launcher will automatically resize your active command shell window buffer configuration to fullscreen, set the workspace context title, and initialize a clean interface seamlessly.

---

## ⌨️ Interface Control Bindings (Hotkeys Matrix)

The application handles grid matrix navigation through precise, custom event capturing overrides:

| Keyboard Input Key | Visual Interface Action | Operational Context Routing |
| :--- | :--- | :--- |
| **`[Arrow Left / Right]`** | Shift Horizontal Column Focus | Jumps cursor focus selection between `TO DO` $\leftrightarrow$ `IN PROGRESS` $\leftrightarrow$ `DONE`. |
| **`[Arrow Up / Down]`** | Navigate Focused Column List | Moves task selection selector up/down. |
| **`[Space] / [Enter]`** | Advance Task Status Forward | Shifts the active card forward: `TODO` $\rightarrow$ `IN_PROGRESS` $\rightarrow$ `DONE`. |
| **`[Backspace]`** | Regress Task Status Backward | Cascades the active card backward: `DONE` $\rightarrow$ `IN_PROGRESS` $\rightarrow$ `TODO`. |
| **`[N]`** | Instantiate New Record Creation | Opens an overlay screen input dialogue model layout canvas. |
| **`[E]`** | Modify Targeted Task Details | Pre-loads current record string state into input buffers for updates. |
| **`[D]`** | Trigger Deletion Event Logic | Intercepts processing with an active delete confirmation warning prompt. |
| **`[Q]`** | Terminate Execution Loop | Cleanly flushes graphic buffers and exits back to native host shell. |

---

## 💻 Developer Guide (Compilation & Tests Setup)

If you plan to modify the source code, review components architecture, or execute test assertions locally, utilize the standard project build system pipeline.

### Prerequisites Toolchain
Ensure you have an operational **C++20** standard compliance compiler environment installed (e.g., `GCC/MinGW` via MSYS2 profile tracking) alongside `CMake` build orchestration blocks.

### Build and Compilation Pipeline

```bash
# 1. Regenerate optimization configuration schemas targeted to Release mode
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build the targeted interactive binary driver workspace payload
cmake --build build --target kanban_board
```

### 🧪 Executing Unit Test Harness Contexts

Core state alterations, backward workflow regressions, and model schema data mutations are rigorously locked behind test suites powered by the **Catch2** verification framework:

```bash
# Compile and build the decoupled test executable target
cmake --build build --target run_tests

# Run the test validation test cases assertions layout
./build/run_tests
```

---

## 📄 Automated Documentation (Doxygen)

The entire codebase header infrastructure utilizes standardized documentation blocks. To extract and compile the interactive documentation web tree locally:

1. Ensure `doxygen` is available within your active system environment variables.
2. Run the extraction execution signal from the root directory layout:
```bash
doxygen
```
3. Open the resulting target file `doc/html/index.html` using any preferred web browser canvas to review classes, methods, and structural block dependencies maps.

---

## 🏗️ Technical Architecture Highlights

* **Fluid 33% Split Topology Grid:** Replaced erratic absolute layout bindings with dynamic fluid matrix equations (`(Terminal::Size().dimx - 2) / 3`) guaranteeing equal, zero-bleed presentation viewports.
* **Hard Word-Wrapping Engine:** Custom string tokenization algorithms split continuous unbroken string structures safely inside task box boundaries, preventing layout distortions.
* **Insulated State Isolation Design (`AppState`):** Presentation renderer pipelines are separated from navigation logic coordinate configurations, allowing easy data flushes.
* **Doxygen Standardization:** Code infrastructure structures and header interfaces are documented to be fully compatible with Doxygen automatic HTML extraction trees.

---

## ✍️ Author & Course Context

* **Developer:** Wiktor Trybus ([Veexeq](https://github.com/Veexeq))
* **Academic Institution:** AGH University of Science and Technology (*Akademia Górniczo-Hutnicza w Krakowie*)
* **Faculty:** Faculty of Computer Science (*Wydział Informatyki*)
* **Course Assignment:** Final Laboratory Software Project for the *Programming in C++* course
* **Timeline Build:** Summer Semester 2026