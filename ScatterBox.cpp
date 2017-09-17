#include "ScatterBox.hpp"

#include <array>
using std::array;

#include <string>
using std::to_string;

#include <cmath>
using std::round;
using std::abs;
using std::sin;
using std::cos;
using std::sqrt;

#include <curses.h>

#include <vector>
using std::vector;



//
// Geometry stuff
//

array<double,3> normalized(array<double,3> v){
    double norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    array<double,3> result;
    result[0] = v[0] / norm;
    result[1] = v[1] / norm;
    result[2] = v[2] / norm;
    return result;
}

array<double,3> rotate(array<double,3> p, array<double,3> axis, double angle){
    // Hardcoded rotation matrix multiplication
    array<double,3> axisn = normalized(axis);

    const double CC  = 1-cos(angle);
    const double C   = cos(angle);
    const double S   = sin(angle);
    const double ux  = axisn[0];
    const double ux2 = ux*ux;
    const double uy  = axisn[1];
    const double uy2 = uy*uy;
    const double uz  = axisn[2];
    const double uz2 = uz*uz;

    array<double,3> result;
    result[0] = (C+ux2*CC)     *p[0] + (ux*uy*CC-uz*S)*p[1] + (ux*uz*CC+uy*S)*p[2];
    result[1] = (uy*uz*CC+uz*S)*p[0] + (C+uy2*CC)     *p[1] + (uy*uz*CC-ux*S)*p[2];
    result[2] = (uz*ux*CC-uy*S)*p[0] + (ux*uy*CC+ux*S)*p[1] + (C+uz2*CC)     *p[2];
    return result;
}


//
// Scatterbox
//

ScatterBox::ScatterBox(WINDOW *scatter_win, std::mutex &new_drawing_mutex)
    : drawing_mutex(new_drawing_mutex) {
    win = scatter_win;
    draw();
}

struct pixel_cluster {
    bool pixels[2*4];
};

wchar_t braillify(pixel_cluster pxc){
    // Pixels add the following values to the base braille
    // char:
    //
    //     0x01  0x08
    //     0x02  0x10
    //     0x04  0x20
    //     0x40  0x80

    wchar_t braille_char = 0x2800;

    if (pxc.pixels[0 * 2 + 0]) braille_char += 0x01;
    if (pxc.pixels[0 * 2 + 1]) braille_char += 0x08;
    if (pxc.pixels[1 * 2 + 0]) braille_char += 0x02;
    if (pxc.pixels[1 * 2 + 1]) braille_char += 0x10;
    if (pxc.pixels[2 * 2 + 0]) braille_char += 0x04;
    if (pxc.pixels[2 * 2 + 1]) braille_char += 0x20;
    if (pxc.pixels[3 * 2 + 0]) braille_char += 0x40;
    if (pxc.pixels[3 * 2 + 1]) braille_char += 0x80;

    return braille_char;
}


void ScatterBox::add_horizontal_rotation(double angle){
    // That's a rotation around the current Y axis (the screen one,
    // not the sim one).
    screen_x = rotate(screen_x, screen_y, angle);
    draw();
}

void ScatterBox::add_vertical_rotation(double angle){
    // Similar
    screen_y = rotate(screen_y, screen_x, angle);
    draw();
}

void ScatterBox::add_horizontal_translation(double dx){
    offset_x += dx;
    draw();
}

void ScatterBox::add_vertical_translation(double dx){
    offset_y += dx;
    draw();
}

// Projects a point in the plane at unit distance from origin and in
// the current angles θ, φ, dumps the result in X, Y.
void ScatterBox::project_point(array<double,3> p, double& X, double& Y){
    // Get X, Y as dot product (projections) on the screen unit vectors.
    X = p[0]*screen_x[0] + p[1]*screen_x[1] + p[2]*screen_x[2];
    Y = p[0]*screen_y[0] + p[1]*screen_y[1] + p[2]*screen_y[2];
}

void ScatterBox::draw() {

    drawing_mutex.lock();

    int width, height;
    getmaxyx(win, height, width);
    werase(win);

    if (width > 10 && height > 4){ // Else too small to draw

        box(win, 0, 0);

        // Usable area is total area minus the borders.
        int H = height - 2;
        int W = width - 2;

        vector<pixel_cluster> braille_matrix(H*W);

        pixel_cluster zeroed_cluster = {{false, false,
                                         false, false,
                                         false, false,
                                         false, false}};

        std::fill(braille_matrix.begin(), braille_matrix.end(), zeroed_cluster);

        // Hit the pixels, will translate later to braille.
        for(auto d : data){

            double X, Y;
            project_point(d, X, Y);
            X += offset_x;
            Y += offset_y;
            X *= zoom_factor;
            Y *= -1 * zoom_factor; // The screen coords are flipped!

            if(std::abs(X) > 1 || std::abs(Y) > 1){
                continue; // Out of screen
            }

            // Pixels map in [-1,1], they span 2 units of length in total.

            // Full pixel pos
            int full_pix_x = std::round((X + 1) * (W * 2 - 1) / 2.0);
            int full_pix_y = std::round((Y + 1) * (H * 4 - 1) / 2.0);

            // Belonging pixel cluster:
            int pix_x = full_pix_x / 2;
            int pix_y = full_pix_y / 4;

            // Pixel offset inside the pixel cluster:
            int off_x = full_pix_x % 2;
            int off_y = full_pix_y % 4;

            braille_matrix[pix_y * W + pix_x].pixels[off_y * 2 + off_x] = true;
        }

        // Translate on the fly the 2*4 groups of pixels to braille
        for (int pix_y = 0; pix_y < H; ++pix_y) {
            for (int pix_x = 0; pix_x < W; ++pix_x) {

                mvwprintw(win, pix_y + 1, pix_x + 1, // Avoid border!
                          "%lc", braillify(braille_matrix[pix_y * W + pix_x]));
            }
        }

        // Upper left data
        if(show_info){
            mvwprintw(win, 1+0, 1, ("zoom: ×" + to_string(zoom_factor)).c_str()  );
            if(data.size() == 0)
                mvwprintw(win, 1+1, 1, " -- No data to show --");
            else
                mvwprintw(win, 1+1, 1, ("Showing " + to_string(data.size()) + " points").c_str());
        }

    } // If big enough to draw

    wrefresh(win);
    drawing_mutex.unlock();

}

void ScatterBox::change_data(vector<array<double,3>> newdata){
    data = newdata;
    draw();
}

void ScatterBox::zoom(double zoom_change){
    zoom_factor += zoom_change;
    if (zoom_factor <= 0) zoom_factor = MIN_ZOOM;
    draw();
}

void ScatterBox::reset_view(){
    screen_x = array<double,3>({{1,0,0}}); // Unit vectors of the 2D camera in the simulation.
    screen_y = array<double,3>({{0,1,0}});
    offset_x = 0;
    offset_y = 0;
    zoom_factor = 1;
    draw();
}
