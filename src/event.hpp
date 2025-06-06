#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>

enum class EventID {
    REFRESH_PROFILE,
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

class DeletePhotoEvent final: public Event {
  public:
    DeletePhotoEvent() noexcept;
    EventID get_type() override;
};

class RefreshProfileEvent final: public Event {
  public:
    RefreshProfileEvent() noexcept;
    EventID get_type() override;
};

#endif