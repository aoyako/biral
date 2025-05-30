#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>

enum EventID {
    SET_PHOTO,
    DELETE_PHOTO,
};

class Event {
  public:
    virtual EventID get_type() = 0;
    virtual ~Event() = default;
};

class SetPhotoEvent final: public Event {
  private:
    const std::string photo_path;

  public:
    SetPhotoEvent(const std::string &path) noexcept;
    EventID get_type() override;
    std::string get_photo_path() noexcept;
};

#endif