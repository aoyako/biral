#ifndef CONFIG_HPP
#define CONFIG_HPP

class Config {
  private:
    int verbosity_level_{0};
    int process_timeout_{10};

  public:
    Config(int verbosity_level, int process_timeout) noexcept;

    int get_verbosity_level() const noexcept;
    int get_timeout() const noexcept;
};

#endif