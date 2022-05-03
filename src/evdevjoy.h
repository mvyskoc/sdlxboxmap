#ifndef __EVDEVJOY_H
#define __EVDEVJOY_H

#include <string>
#include <string_view>
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <libevdev/libevdev.h>

namespace evdevjoy {

class ControllerMapping;
class EvdevJoystick;


enum class ControllerButton
{
    INVALID = -1,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_BACK,
    BUTTON_GUIDE,
    BUTTON_START,
    BUTTON_LEFTSTICK,
    BUTTON_RIGHTSTICK,
    BUTTON_LEFTSHOULDER,
    BUTTON_RIGHTSHOULDER,
    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,
    BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    BUTTON_PADDLE1,  /* Xbox Elite paddle P1 */
    BUTTON_PADDLE2,  /* Xbox Elite paddle P3 */
    BUTTON_PADDLE3,  /* Xbox Elite paddle P2 */
    BUTTON_PADDLE4,  /* Xbox Elite paddle P4 */
    BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    BUTTON_MAX,
    AXIS_LEFTX,
    AXIS_LEFTY,
    AXIS_RIGHTX,
    AXIS_RIGHTY,
    AXIS_TRIGGERLEFT,
    AXIS_TRIGGERRIGHT,
    AXIS_MAX
};

namespace HatMask {
    const int UP = 0x1;
    const int RIGHT = 0x2;
    const int DOWN = 0x4;
    const int LEFT = 0x8;
}

enum class BindType
{
    BINDTYPE_NONE,
    BINDTYPE_BUTTON,
    BINDTYPE_AXIS,
    BINDTYPE_HAT
};

enum class ControllerAxisType
{
  FULL_AXIS,
  HALF_AXIS_POSITIVE,
  HALF_AXIS_NEGATIVE
};

struct ButtonBinding
{
    BindType input_type;
    union 
    {
        int button;
        struct {
            int axis;
            ControllerAxisType axis_type;
            bool invert_input;
        };

        struct {
            int hat;
            int hat_mask;
        };
    } input;

    BindType output_type;     // BINDTYPE_AXIS or BINDTYPE_BUTTON
    union
    {
        ControllerButton button;
        struct {
            ControllerButton axis;
            ControllerAxisType axis_type;
        };
    } output;
};

//////////////////////////////////////////////////////////////////////////
// ControllerMapping
//////////////////////////////////////////////////////////////////////////

class ControllerMapping {
  public:

  typedef enum {
    PRIORITY_DEFAULT,
    PRIORITY_API,
    PRIORITY_USER
  } priority_t;

    std::string guid;
    std::string name;
    std::string platform;
    priority_t priority;
    std::map<std::string, std::string> hints;
    std::map<ControllerButton, ButtonBinding> button_binding;
    
    ControllerMapping() : priority(PRIORITY_DEFAULT) {}
    ControllerMapping(std::string const &mapping_str, priority_t priority = PRIORITY_DEFAULT);
    ControllerButton get_button_from_string(const std::string &name);

  protected:
    void parse_config_element(const std::string &item);
    void add_button_binding(const std::string &button_name, 
        const std::string &button_def);
};

//////////////////////////////////////////////////////////////////////////
// SDLJoyMapping
//////////////////////////////////////////////////////////////////////////

class SDLJoyMapping {
  public:

    SDLJoyMapping();
    // Add mapping only of the corresponding platform
    void add_sdl_mapping(ControllerMapping const &mapping);
    void add_sdl_mapping(std::istream &iss, 
        ControllerMapping::priority_t priority = ControllerMapping::PRIORITY_API);
    
    void add_default_mapping() {
      add_internal_mappings();
      add_user_mappings();
    }

    void add_internal_mappings();

    // Add mapping into the database from environment database SDL_GAMECONTROLLERCONFIG
    void add_user_mappings();

    ControllerMapping* get_mapping(std::string const &guid);
    ~SDLJoyMapping();
  protected:
    std::unordered_map<std::string, ControllerMapping> mapping_db;

  private:
    // Disable copy constructor and assign operator
    SDLJoyMapping(const SDLJoyMapping&) = delete;
    SDLJoyMapping& operator=(const SDLJoyMapping&) = delete;
};

typedef uint8_t joy_guid_t[16];

struct EventType {
    unsigned int type;
    unsigned int code;
    EventType(unsigned int t, unsigned int c) : type(t), code(c) {};
    std::string_view get_name() const;
};

struct HatEventType {
    EventType x;
    EventType y;
};

struct EventButtonBinding
{
    const EventType *event;
    const ButtonBinding *bind;

    operator bool() const {
      return (event != nullptr) && (bind != nullptr);
    };
};

class EvdevJoystick
{
  public:
    std::vector<EventType> buttons;
    std::vector<HatEventType> hats;
    std::vector<EventType> axes;
    std::string devname;    //Device path
    
    // Returns list of vector of event devices 
    static std::vector<std::string> get_event_devices();

    EvdevJoystick(std::string const &devname);
    std::string get_guid();
    void get_guid(joy_guid_t &guid);

    // The returned name is valid until EvdevJoystic is released
    std::string_view get_name();

    /* Set mapping invalidate all existing pointers in EventButtonBinding */
    void set_mapping(ControllerMapping const &mapping);
    bool set_mapping(SDLJoyMapping &joy_mapping);
    EventButtonBinding get_event_binding(ControllerButton button);
    ~EvdevJoystick();

  protected:
    ControllerMapping mapping;
    struct libevdev *evdev = nullptr;
    void get_button_settings();
    void get_hat_settings();
    void get_axes_settings();

  private:
    // Disable copy constructor and assign operator
    EvdevJoystick(const EvdevJoystick&) = delete;
    EvdevJoystick() = delete;
    EvdevJoystick& operator=(const EvdevJoystick&) = delete;
};

} //namespace evdevjoy
#endif