#include "reactor.h"

int main() {
    Reactor reactor(8080);
    reactor.run();
    return 0;
}