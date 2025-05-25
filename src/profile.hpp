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
#include "config.hpp"

class Profile {
  public:
    Profile(Config cfg) noexcept;

    void tick();
    void init();

  private:
    using Object = td::td_api::object_ptr<td::td_api::Object>;
    using Request = td::td_api::object_ptr<td::td_api::Function>;

    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};
    std::uint64_t current_query_id_{0};
    bool need_restart_{false};
    bool are_authorized_{false};
    bool has_set_photo_{false};
    Config config_;

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
    void Profile::on_authorization_state_ready();

};

#endif PROFILE_HPP