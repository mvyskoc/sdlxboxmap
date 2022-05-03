#ifndef _SDLXBOXMAP_H_INCLUDED__
#define _SDLXBOXMAP_H_INCLUDED__

#include <memory>
#include <string_view>
#include <cxxopts.hpp>
#include <stdexcept>
#include "evdevjoy.h"


namespace sdlxboxmap {

typedef std::unique_ptr<evdevjoy::EvdevJoystick> t_uptr_evdevjoystick;

class MainAppException : public std::runtime_error
{
  public:
    explicit MainAppException(const std::string& msg);
    virtual ~MainAppException() throw () {}
};

class MainApp
{
  public:
    // Maximum size of buffer and command lenght e.g.: <MAP_BUTTON:dpad_x> ,...*/
    static const size_t BUFFER_SIZE = 50;

    evdevjoy::SDLJoyMapping joymap;
    enum class e_mapping_result{ FOUND, NOT_FOUND, UNSUPPORTED };

    MainApp();
    void arg_parse(int argc, char* argv[]);
    void find_gamepads();
    void init_gamepads(
        const std::vector<std::string> &guid,
        const std::vector<std::string> &filter,
        std::vector<t_uptr_evdevjoystick> &gamepads);
    void replace_mapping(evdevjoy::EvdevJoystick &gamepad, const std::string &tpl_filename, 
        const std::string &out_filename);

    void make_file_substitution(cxxopts::ParseResult &parsed_args);
    void sreplace_mapping(evdevjoy::EvdevJoystick &gamepad, 
        std::istream &is, std::ostream &os);
    MainApp::e_mapping_result get_mapping_value(evdevjoy::EvdevJoystick &gamepad,
        std::string_view map_command, std::string &result);
    MainApp::e_mapping_result map_evdev(evdevjoy::EvdevJoystick &gamepad,
        std::string &result);
    MainApp::e_mapping_result map_abs(evdevjoy::EvdevJoystick &gamepad, 
        std::string_view button_name, std::string &result);
    MainApp::e_mapping_result map_button(evdevjoy::EvdevJoystick &gamepad, 
        std::string_view button_name, std::string &result);
    MainApp::e_mapping_result axismap(evdevjoy::EvdevJoystick &gamepad,
        std::string_view button_name, std::string &result);

  protected:
    std::unique_ptr<cxxopts::Options> init_arg_parser();
  private:
    // Disable copy constructor and assign operator
    MainApp(const MainApp&) = delete;
    MainApp& operator=(const MainApp&) = delete;
};

} // namespace sdlxboxmap
#endif