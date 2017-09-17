#include "MsgBox.hpp"
#include <curses.h>

#include <algorithm>
using std::remove;

#include <vector>
using std::vector;

#include <sstream>
#include <string>
using std::string;
using std::to_string;

#include <cmath>
using std::ceil;

MsgBox::MsgBox(WINDOW* msg_win, std::mutex& new_drawing_mutex)
    : drawing_mutex(new_drawing_mutex){
    win = msg_win;
    draw();
}

vector<string> wrap(string msg, int width) {
    vector<string> lines;
    int chunks = ceil(msg.size() / (double)width);
    for (int i = 0; i < chunks; i++) {
        lines.push_back(msg.substr(i * width, width));
    }
    return lines;
}

// "foo bar baz boo"     →    "foo bar b…"
string truncate(string msg, int width){
    if(width < 0) return "";
    if(msg.size() > width)
        return msg.substr(0,width-1) + "…";
    else
        return msg;
}

void MsgBox::draw() {
    drawing_mutex.lock();

    int width, height;
    getmaxyx(win, height, width);
    werase(win);

    if (width > 3 && height > 3){ // Else too small to draw

        box(win, 0, 0);

        // Just shove to screen the last messages
        int Nmessages = height - 2; // The two borders!

        for(int i = 0; i < Nmessages; i++){
            // Mind the two borders when computing the width
            try {
                string msg = scrollback_buffer.at(last_msg_idx - i);
                msg = truncate(msg, width - 2); // The borders!
                // Color special messages in different colors.
                if (msg.substr(0,8) == "Termsim:"){
                    wattron(win, A_BOLD);
                    mvwprintw(win, height - 2 - i, 1, msg.substr(0,8).c_str());
                    wattroff(win, A_BOLD);
                    mvwprintw(win, height - 2 - i, 1+8, msg.substr(8).c_str());
                }
                else if (msg.substr(0,5) == "Info:"){
                    wattron(win, A_BOLD);
                    wattron(win, COLOR_PAIR(1));
                    mvwprintw(win, height - 2 - i, 1, msg.substr(0,5).c_str());
                    wattroff(win, COLOR_PAIR(1));
                    wattroff(win, A_BOLD);
                    mvwprintw(win, height - 2 - i, 1+5, msg.substr(5).c_str());
                }
                else if (msg.substr(0,8) == "Warning:"){
                    wattron(win, A_BOLD);
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, height - 2 - i, 1, msg.substr(0,8).c_str());
                    wattroff(win, COLOR_PAIR(2));
                    wattroff(win, A_BOLD);
                    mvwprintw(win, height - 2 - i, 1+8, msg.substr(8).c_str());
                }
                else if (msg.substr(0,6) == "Error:"){
                    wattron(win, A_BOLD);
                    wattron(win, COLOR_PAIR(3));
                    mvwprintw(win, height - 2 - i, 1, msg.substr(0,6).c_str());
                    wattroff(win, COLOR_PAIR(3));
                    wattroff(win, A_BOLD);
                    mvwprintw(win, height - 2 - i, 1+6, msg.substr(6).c_str());
                }
                else {
                    mvwprintw(win, height - 2 - i, 1, msg.c_str());
                }
            } catch (std::out_of_range) {
                // Don't print anything, not enough lines of output yet.
            }
        }//for Nmessages

    }//if enough big

    // Draw scroll-lock and num of msgs
    if(!autoscroll)
        mvwprintw(win, 0, width-12, "SCROLL LOCK");

    string msg_indicator =
        to_string(last_msg_idx+1) +
        "/" +
        to_string(scrollback_buffer.size());

    mvwprintw(win, 0, 1, msg_indicator.c_str());

    mvwprintw(win, height-2, 0, "|"); // Last msg indicator

    refresh();
    wrefresh(win);
    drawing_mutex.unlock();
}

void trim_whitespace(string& str){

    // str.erase( std::remove( str.begin(), str.end(), '\n' ), str.end() );
    auto whitespace = "\n\t\r ";
    auto start = str.find_first_not_of(whitespace);
    auto end = str.find_last_not_of(whitespace);
    str = str.substr(start, 1+end-start);
}

std::vector<std::string> split_string(std::string str, char delim) {

    std::stringstream sstr(str);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(sstr, segment, delim)) {
        seglist.push_back(segment);
    }

    return seglist;
}

// This supposes that the msg has no escape chars anywhere except at
// the end, where one can have either \n or \r. If \r and the last one
// was also an \r, overwrite it.
void MsgBox::insert_msg(std::string str){
    static bool last_was_cr = false;
    auto L = str.size();
    // Remember to trim the special character.
    if (str.back() == '\n'){
        scrollback_buffer.push_back(str.substr(0,L-1));
        last_was_cr = false;
    }
    else if (str.back() == '\r'){
        if (last_was_cr)
            scrollback_buffer.back() = str.substr(0,L-1);
        else
            scrollback_buffer.push_back(str.substr(0,L-1));
        last_was_cr = true;
    }
    else {
        scrollback_buffer.push_back(str);
        last_was_cr = false;
    }

    if(autoscroll) last_msg_idx = scrollback_buffer.size()-1;
    draw();
}

void MsgBox::scroll_down(){
    if (last_msg_idx < scrollback_buffer.size()-1) ++last_msg_idx;
    draw();
}

void MsgBox::scroll_up(){
    if (last_msg_idx > 0) --last_msg_idx;
    draw();
}

void MsgBox::go_to_beginning() {
    last_msg_idx = 0;
    draw();
}

void MsgBox::go_to_end() {
    last_msg_idx = scrollback_buffer.size()-1;
    draw();
}

void MsgBox::toggle_autoscroll() {
    if(!autoscroll) go_to_end(); // Returning from it.
    autoscroll = !autoscroll;
    draw();
}
