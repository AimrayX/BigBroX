#include "UCIHandler.hpp"
#include "engine.hpp"

int main() {

    UCIHandler uciHandler;

    while (uciHandler.state != 1) {
        uciHandler.loop();
    }
    

    return 0;
}
