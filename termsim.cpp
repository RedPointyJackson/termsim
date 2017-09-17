
#include <array>
using std::array;

#include <iostream>

#include <stdio.h>

#include <string>
using std::string;

#include <thread>
using std::thread;

#include "Tui.hpp"

#include <unistd.h>

#include <vector>
using std::vector;


Tui tui;
string command;

// Keeps reading characters from `file` until \r or \n is found.
class CouldntRead{};
string read_sentence(FILE* file){
    vector<char> characters;
    char last_char = 'a';
    while(last_char != '\n' && last_char != '\r'){
        size_t chars_readed = fread(&last_char, 1, sizeof(char), file);
        if(chars_readed != 1) throw CouldntRead();
        characters.push_back(last_char);
    }
    return string(characters.begin(), characters.end());
}

array<double,3> read_point(string msg){
    double x,y,z;
    sscanf(msg.c_str(), "%lf %lf %lf\n", &x, &y, &z);
    return array<double,3>({{x,y,z}});
}

void readerjob(){

    FILE* pipe = popen(command.c_str(), "r");
    std::setbuf(pipe, NULL);

    // Will be savagely closed with ^C, no need to pclose or break the
    // while.
    while(true){

        if(tui.exited()) break;

        string msg;
        try {
            msg = read_sentence(pipe);
        }
        catch (CouldntRead &e) {
            tui.msg_box->insert_msg("Termsim: Can't read more from pipe, closing it.");
            pclose(pipe);
            return;
        }

        if(string(msg) == string("END\n")){
            tui.msg_box->insert_msg("Termsim: END without DATA.");
        }

        /////////////////////////////
        //        DATA READ        //
        /////////////////////////////
        if(string(msg) == string("DATA\n")){

            vector<array<double,3>> new_data;

            // Read a line of data until END or failure.
            while(true){
                msg = read_sentence(pipe);
                if(msg=="END\n") break;
                if(msg=="DATA\n"){
                    tui.msg_box->insert_msg("Termsim: DATA before END. Aborting data collection.");
                    break;
                }
                new_data.push_back(read_point(msg));
            }

            tui.scatter_box->change_data(new_data);
            tui.scatter_box->draw();

        /////////////////////////////
        //        MSG READ         //
        /////////////////////////////
        } else {
            tui.msg_box->insert_msg(msg);
        }

    }

}

int main(int argc, char** argv){

    for(int i=1; i<argc; ++i){
        command += argv[i];
        command += " ";
    }
    command += "2>&1";
    tui.msg_box->insert_msg("Termsim: Executing \"" + command + "\"");
    thread reader(readerjob);
    tui.main_loop();
}
