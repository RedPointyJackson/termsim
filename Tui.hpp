#ifndef TUI_H
#define TUI_H

#include <memory>
#include <curses.h>
#include "MsgBox.hpp"
#include "ScatterBox.hpp"

#include <mutex>

class Tui {
private:
    int term_width, term_height;
    int msg_box_height = 7;
    bool curses_was_initialized = false;
    WINDOW* msg_box_win = nullptr;
    WINDOW* scatter_box_win = nullptr;
    bool has_exited = false;
public:
    Tui();
    ~Tui();
    std::mutex drawing_mutex;
    std::unique_ptr<MsgBox> msg_box;
    std::unique_ptr<ScatterBox> scatter_box;
    void main_loop();
    bool exited() {return has_exited;}
};

#endif
