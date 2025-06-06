#include "profile.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>

int main() {
    Config cfg(0, 10);
    Profile refresher(cfg);
    refresher.init();

    auto begin = std::chrono::high_resolution_clock::now();
    auto begin_refresh = std::chrono::high_resolution_clock::now();
    while (true) {
        auto end = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(end - begin_refresh).count() >= 10) {
            refresher.send_event(std::make_unique<RefreshProfileEvent>());
            begin_refresh = end;
        }
        
        if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() >= 60) {
            // refresher.send_event(std::make_unique<SetPhotoEvent>("2.png"));
            refresher.send_event(std::make_unique<DeletePhotoEvent>());
            begin = end;
        }

        refresher.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}