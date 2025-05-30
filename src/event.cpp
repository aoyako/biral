#include "event.hpp"

SetPhotoEvent::SetPhotoEvent(const std::string &path) noexcept : photo_path(path) {
}

EventID SetPhotoEvent::get_type() {
    return SET_PHOTO;
}

std::string SetPhotoEvent::get_photo_path() noexcept {
    return photo_path;
}