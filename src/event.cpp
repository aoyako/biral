#include "event.hpp"

SetPhotoEvent::SetPhotoEvent(const std::string &path) noexcept : photo_path(path) {
}

EventID SetPhotoEvent::get_type() {
    return EventID::SET_PHOTO;
}

std::string SetPhotoEvent::get_photo_path() noexcept {
    return photo_path;
}

DeletePhotoEvent::DeletePhotoEvent() noexcept {
}

EventID DeletePhotoEvent::get_type() {
    return EventID::DELETE_PHOTO;
}

RefreshProfileEvent::RefreshProfileEvent() noexcept {
}

EventID RefreshProfileEvent::get_type() {
    return EventID::REFRESH_PROFILE;
}
