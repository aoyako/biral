#include "profile.hpp"
#include "config.hpp"

int main() {
    Config cfg(0, 10);
    Profile refresher(cfg);
    refresher.init();
    refresher.send_event(std::make_unique<SetPhotoEvent>("another/path.png"));
    
    for (int i = 0; i<1000000; ++i) {
        refresher.tick();
    }
    return 0;
}