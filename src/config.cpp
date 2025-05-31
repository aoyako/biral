#include "config.hpp"

Config::Config(int verbosity_level, int process_timeout) noexcept
    : verbosity_level_(verbosity_level_), process_timeout_(process_timeout_) {
}

int Config::get_verbosity_level() const noexcept {
    return verbosity_level_;
}
int Config::get_timeout() const noexcept {
    return process_timeout_;
}