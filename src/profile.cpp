#include "profile.hpp"
#include "config.hpp"

Profile::Profile(Config cfg) : config_(cfg) {
    td::ClientManager::execute(td::td_api::make_object<td::td_api::setLogVerbosityLevel>(config_.get_verbosity_level()));
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
    } else if (!has_set_photo_) {
        has_set_photo_ = true;
        // set_profile_photo("images.jpg");
        send_query(td::td_api::make_object<td::td_api::getMe>());
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
            auto photo_id = u.profile_photo_->id_;
            send_query(td::td_api::make_object<td::td_api::deleteProfilePhoto>(photo_id));
            std::cout << "deleted\n";
        } else {
            std::cout << "No profile photo to delete\n";
        }
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