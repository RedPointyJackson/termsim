#ifndef SCATTERBOX_H
#define SCATTERBOX_H

#include <array>
#include <curses.h>
#include <mutex>
#include <vector>

// AXES IN THE SCREEN:
//
//          |Z
//          |                   Physicists' spherical coords.
//          |                   θ: angle with Z
//          |                   φ: angle of proy in XY with X
//          |-----------
//         /           Y
//      X /
//


class ScatterBox {
private:
    const double MIN_ZOOM = 0.1;

    std::vector<std::array<double,3>> data; // Supposed to be in [-1,1]. x,y,z in triples.

    double zoom_factor = 1;
    double offset_x = 0; // To move the sim
    double offset_y = 0;
    WINDOW* win = nullptr;
    std::mutex& drawing_mutex;
    std::array<double,3> screen_x = {{0,1,0}}; // Unit vectors of the 2D camera in the simulation.
    std::array<double,3> screen_y = {{1,0,0}}; // Start looking at XY.
    bool show_info = true;
    void project_point(std::array<double,3> p, double& X, double& Y);
public:
    void add_vertical_rotation(double angle);
    void add_horizontal_rotation(double angle);
    void add_vertical_translation(double dx);
    void add_horizontal_translation(double dx);
    bool show_axes = true;
    ScatterBox(WINDOW* msg_win, std::mutex& new_drawing_mutex);
    void change_data(std::vector<std::array<double,3>> newdata);
    void draw();
    void zoom(double zoom_change);
    void reset_view();
    void toggle_info() {show_info = !show_info; draw();};
};

#endif
