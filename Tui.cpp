#include <locale>
#include <iostream> // DEBUG
#include <curses.h>
#include "Tui.hpp"
#include "MsgBox.hpp"

Tui::Tui() {
    std::cout << "Initializing Tui\n";
    if (curses_was_initialized)
        throw std::runtime_error("Trying to initalize twice curses.");

    // As the man page suggests:
    setlocale(LC_CTYPE, "");
    initscr(); curses_was_initialized = true;
    cbreak();
    noecho();
    nonl();
    intrflush(msg_box_win, FALSE);
    curs_set(0);

    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);


    getmaxyx(stdscr, term_height, term_width);

    msg_box_win =
        newwin(msg_box_height, term_width, term_height - msg_box_height, 0);
    scatter_box_win =
        newwin(term_height - msg_box_height, term_width, 0, 0);

    wtimeout(msg_box_win, -1); // We read keys from there, set
                               // as locking

    msg_box.reset(new MsgBox(msg_box_win, drawing_mutex));
    scatter_box.reset(new ScatterBox(scatter_box_win, drawing_mutex));
}

Tui::~Tui() {
    delwin(msg_box_win);
    delwin(scatter_box_win);
    endwin();
    std::cout << "Closing Tui\n";
}

void Tui::main_loop() {

    const double ROT_STEP = 0.1;
    const double TRANS_STEP = 0.1;
    const double ZOOM_STEP = 0.1;

    while(true){ // Until ^C

        // Redraw if resize
        int old_term_height = term_height;
        int old_term_width = term_width;
        getmaxyx(stdscr, term_height, term_width);

        if (old_term_width != term_width || old_term_height != term_height) {
            mvwin(msg_box_win, term_height - msg_box_height, 0);
            wresize(msg_box_win, msg_box_height, term_width);
            msg_box->draw();

            wresize(scatter_box_win, term_height - msg_box_height, term_width);
            scatter_box->draw();
        }

        std::string key = std::string(key_name(wgetch(msg_box_win)));

        // Message box things
        if (key == "p" || key == "^P")      {msg_box->scroll_up();}
        else if (key == "n" || key == "^N") {msg_box->scroll_down();}
        else if (key == " ")                {msg_box->toggle_autoscroll();}
        else if (key == "g")                {msg_box->go_to_beginning();}
        else if (key == "G")                {msg_box->go_to_end();}

        // Scatter plot
        else if (key == "j")                {scatter_box->add_vertical_rotation(-ROT_STEP);}
        else if (key == "k")                {scatter_box->add_vertical_rotation(+ROT_STEP);}
        else if (key == "l")                {scatter_box->add_horizontal_rotation(-ROT_STEP);} // Just Works™
        else if (key == "h")                {scatter_box->add_horizontal_rotation(+ROT_STEP);}

        else if (key == "J")                {scatter_box->add_vertical_translation(-TRANS_STEP);}
        else if (key == "K")                {scatter_box->add_vertical_translation(+TRANS_STEP);}
        else if (key == "L")                {scatter_box->add_horizontal_translation(+TRANS_STEP);}
        else if (key == "H")                {scatter_box->add_horizontal_translation(-TRANS_STEP);}

        else if (key == "+")                {scatter_box->zoom(+ZOOM_STEP);}
        else if (key == "-")                {scatter_box->zoom(-ZOOM_STEP);}

        else if (key == "r")                {scatter_box->reset_view();}

        else if (key == "i")                {scatter_box->toggle_info();}
        // Misc
        else if (key == "^L")               {msg_box->draw(); scatter_box->draw();}
        // else                                {msg_box->insert_msg("Termsim: Unrecognized key \"" + key + "\"");}

    }

    has_exited = true;

}
