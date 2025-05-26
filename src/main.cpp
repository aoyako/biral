#include "profile.hpp"
#include "config.hpp"

int main() {
    Config cfg(1, 10);
    Profile refresher(cfg);
    refresher.init();
    
    while (true) {
        refresher.tick();
    }
    return 0;
}