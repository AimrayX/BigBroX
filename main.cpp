#include "UCIHandler.hpp"
#include "engine.hpp"

int main() {

    UCIHandler uciHandler;

    while (uciHandler.state != 1) {
        uciHandler.start();
    }
    

    return 0;
}