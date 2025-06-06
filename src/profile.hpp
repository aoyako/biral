#ifndef PROFILE_HPP
#define PROFILE_HPP

#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <td/telegram/td_api.hpp>
#include <vector>
#include <queue>
#include <optional>
#include "config.hpp"
#include "event.hpp"

class Profile {
  public:
    Profile(const Config &cfg) noexcept;

    void tick();
    void init();

    void send_event(std::unique_ptr<Event> event);

  private:
    using Object = td::td_api::object_ptr<td::td_api::Object>;
    using Request = td::td_api::object_ptr<td::td_api::Function>;

    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};
    std::uint64_t current_query_id_{0};
    std::optional<std::int64_t> current_photo_id_{};
    bool need_restart_{false};
    bool are_authorized_{false};
    Config config_;
    std::queue<std::unique_ptr<Event>> events;

    void restart();

    void send_query(Request f);

    std::uint64_t next_query_id();

    void process_response(td::ClientManager::Response responsee);

    void set_profile_photo(const std::string &file_path);

    void on_authorization_state(td::td_api::object_ptr<td::td_api::AuthorizationState> state);

    void on_authorization_state_parameters();
    void on_authorization_state_phone_number();
    void on_authorization_state_code();
    void on_authorization_state_password();
    void on_authorization_state_ready();

};

#endif