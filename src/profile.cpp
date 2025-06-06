#include "profile.hpp"
#include "config.hpp"

Profile::Profile(const Config &cfg) noexcept : config_(cfg) {
    td::ClientManager::execute(td::td_api::make_object<td::td_api::setLogVerbosityLevel>(0));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
}

void Profile::init() {
    send_query(td::td_api::make_object<td::td_api::getOption>("version"));
}

void Profile::tick() {
    if (need_restart_) {
        restart();
    } else if (!are_authorized_) {
        process_response(client_manager_->receive(10));
    } else if (!events.empty()) {
        std::unique_ptr<Event> event = std::move(events.front());
        events.pop();
        switch (event->get_type()) {
            case EventID::REFRESH_PROFILE: {
                send_query(td::td_api::make_object<td::td_api::getMe>());
                break;
            }
            case EventID::SET_PHOTO: {
                std::unique_ptr<SetPhotoEvent> set_photo_event(
                    dynamic_cast<SetPhotoEvent*>(event.release())
                );
                std::cout << "set photo was called with " << set_photo_event->get_photo_path()
                        << std::endl;
                set_profile_photo(set_photo_event->get_photo_path());
                send_query(td::td_api::make_object<td::td_api::getMe>());
                current_photo_id_.reset();

                break;
            }
            case EventID::DELETE_PHOTO: {
                std::cout << "delete photo was called" << std::endl;
                if (current_photo_id_.has_value()) {
                    send_query(td::td_api::make_object<td::td_api::deleteProfilePhoto>(current_photo_id_.value()));
                    current_photo_id_.reset();
                }
                send_query(td::td_api::make_object<td::td_api::getMe>());
                break;
            }
            default: {
                std::cerr << "unhandled event type " << static_cast<int>(event->get_type()) << std::endl;
                break;
            }
        }
        // std::unique_ptr<
        // } else if (!has_set_photo_) {
        //     has_set_photo_ = true;
        //     // set_profile_photo("images.jpg");
        //     send_query(td::td_api::make_object<td::td_api::getMe>());
    } else {
        process_response(client_manager_->receive(0.0));
    }
}

void Profile::restart() {
    client_manager_.reset();
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    init();
}

void Profile::send_query(Request f) {
    auto query_id = next_query_id();
    client_manager_->send(client_id_, query_id, std::move(f));
}

std::uint64_t Profile::next_query_id() {
    return ++current_query_id_;
}

void Profile::process_response(td::ClientManager::Response response) {
    if (!response.object) {
        return;
    }

    switch (response.object->get_id()) {
    case td::td_api::user::ID: {
        auto &u = static_cast<const td::td_api::user &>(*response.object);
        if (u.profile_photo_) {
            current_photo_id_.emplace(u.profile_photo_->id_);
        } else {
            current_photo_id_.reset();
        }
            // auto photo_id = u.profile_photo_->id_;
            // send_query(td::td_api::make_object<td::td_api::deleteProfilePhoto>(photo_id));
            // std::cout << "deleted\n";
        // } else {
            // std::cout << "No profile photo to delete\n";
        // }
        break;
    }

    case td::td_api::error::ID: {
        auto &err = static_cast<const td::td_api::error &>(*response.object);
        std::cerr << "Error: " << err.message_ << std::endl;
        if (err.code_ == 401) {
            need_restart_ = true;
        }
        break;
    }

    case td::td_api::updateOption::ID: {
        auto &upd = static_cast<const td::td_api::updateOption &>(*response.object);
        std::cout << "Option “" << upd.name_ << "”: ";

        if (upd.value_->get_id() == td::td_api::optionValueString::ID) {
            auto &v = static_cast<const td::td_api::optionValueString &>(*upd.value_);
            std::cout << v.value_ << std::endl;

            if (upd.name_ == "version") {
                send_query(td::td_api::make_object<td::td_api::getAuthorizationState>());
            }
        }
        break;
    }

    case td::td_api::updateAuthorizationState::ID: {
        auto &upd = static_cast<td::td_api::updateAuthorizationState &>(*response.object);
        on_authorization_state(std::move(upd.authorization_state_));
        break;
    }

    default: {
        std::cerr << "Unhandled auth state: " << response.object->get_id() << std::endl;
        break;
    }
    }
}

void Profile::set_profile_photo(const std::string &file_path) {
    auto file = td::td_api::make_object<td::td_api::inputFileLocal>();
    file->path_ = file_path;

    auto photo = td::td_api::make_object<td::td_api::inputChatPhotoStatic>(std::move(file));

    send_query(td::td_api::make_object<td::td_api::setProfilePhoto>(std::move(photo), false));

    std::cout << "Update photo" << std::endl;
}

void Profile::on_authorization_state_parameters() {
    auto params = td::td_api::make_object<td::td_api::setTdlibParameters>();
    params->database_directory_ = "tdlib";
    params->use_message_database_ = true;
    params->use_secret_chats_ = true;
    params->api_id_ = std::stol(std::getenv("API_ID"));
    params->api_hash_ = std::getenv("API_HASH");
    params->system_language_code_ = "en";
    params->device_model_ = "Desktop";
    params->application_version_ = "1.0";

    send_query(std::move(params));
}

void Profile::on_authorization_state_phone_number() {
    send_query(td::td_api::make_object<td::td_api::setAuthenticationPhoneNumber>(
        std::getenv("PHONE_NUMBER"), nullptr
    ));
}

void Profile::on_authorization_state_code() {
    std::string code;
    std::cout << "Code: ";
    std::cin >> code;
    send_query(td::td_api::make_object<td::td_api::checkAuthenticationCode>(code));
}

void Profile::on_authorization_state_password() {
    std::string pwd;
    std::cout << "Password: ";
    std::cin >> pwd;
    send_query(td::td_api::make_object<td::td_api::checkAuthenticationPassword>(pwd));
}

void Profile::on_authorization_state_ready() {
    are_authorized_ = true;
    std::cout << "Authorized\n";
}

void Profile::on_authorization_state(td::td_api::object_ptr<td::td_api::AuthorizationState> state) {
    switch (state->get_id()) {
    case td::td_api::authorizationStateWaitTdlibParameters::ID: {
        on_authorization_state_parameters();
        break;
    }
    case td::td_api::authorizationStateWaitPhoneNumber::ID: {
        on_authorization_state_phone_number();
        break;
    }
    case td::td_api::authorizationStateWaitCode::ID: {
        on_authorization_state_code();
        break;
    }
    case td::td_api::authorizationStateWaitPassword::ID: {
        on_authorization_state_password();
        break;
    }
    case td::td_api::authorizationStateReady::ID:
        on_authorization_state_ready();
        break;

    case td::td_api::authorizationStateLoggingOut::ID:
    case td::td_api::authorizationStateClosing::ID:
        need_restart_ = true;
        break;

    default:
        std::cerr << "Unhandled auth state: " << state->get_id() << std::endl;
    }
}

void Profile::send_event(std::unique_ptr<Event> event) {
    events.emplace(std::move(event));
}