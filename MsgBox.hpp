#ifndef MSGBOX_H
#define MSGBOX_H

#include <string>
#include <vector>
#include <stdexcept>
#include <curses.h>
#include <mutex>


class MsgBox {
private:
    std::vector<std::string> scrollback_buffer;
    int last_msg_idx = 0;
    WINDOW* win = nullptr;
    std::mutex& drawing_mutex;
    bool autoscroll=true;
public:
    MsgBox(WINDOW* msg_win, std::mutex& new_drawing_mutex);
    void insert_msg(std::string str);
    void draw();
    void scroll_down();
    void scroll_up();
    void go_to_beginning();
    void go_to_end();
    void toggle_autoscroll();
};

#endif
