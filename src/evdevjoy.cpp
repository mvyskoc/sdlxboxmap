#include <stdexcept>
#include <cstring>
#include <sstream>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "evdevjoy"
#endif
#include "logging.h"

#include "platform.h"
#include "evdevjoy.h"
#include "stringext.h"
#include "bitext.h"
#include "joymapdb.h"


using namespace platform;
namespace fs = std::filesystem;

//https://gist.github.com/meghprkh/9cdce0cd4e0f41ce93413b250a207a55
//https://meghprkh.github.io/blog/posts/handling-joysticks-and-gamepads-in-linux/

namespace evdevjoy {


static const std::unordered_map<std::string, ControllerButton> StringToControllerButton = {
    {"a", ControllerButton::BUTTON_A},
    {"b", ControllerButton::BUTTON_B},
    {"x", ControllerButton::BUTTON_X},
    {"y", ControllerButton::BUTTON_Y},
    {"back", ControllerButton::BUTTON_BACK},
    {"guide", ControllerButton::BUTTON_GUIDE},
    {"start", ControllerButton::BUTTON_START},
    {"leftstick", ControllerButton::BUTTON_LEFTSTICK},
    {"rightstick", ControllerButton::BUTTON_RIGHTSTICK},
    {"leftshoulder", ControllerButton::BUTTON_LEFTSHOULDER},
    {"rightshoulder", ControllerButton::BUTTON_RIGHTSHOULDER},
    {"dpup", ControllerButton::BUTTON_DPAD_UP},
    {"dpdown", ControllerButton::BUTTON_DPAD_DOWN},
    {"dpleft", ControllerButton::BUTTON_DPAD_LEFT},
    {"dpright", ControllerButton::BUTTON_DPAD_RIGHT},
    {"misc1", ControllerButton::BUTTON_MISC1},
    {"paddle1", ControllerButton::BUTTON_PADDLE1},
    {"paddle2", ControllerButton::BUTTON_PADDLE2},
    {"paddle3", ControllerButton::BUTTON_PADDLE3},
    {"paddle4", ControllerButton::BUTTON_PADDLE4},
    {"touchpad", ControllerButton::BUTTON_TOUCHPAD},
    {"leftx", ControllerButton::AXIS_LEFTX},
    {"lefty", ControllerButton::AXIS_LEFTY},
    {"rightx", ControllerButton::AXIS_RIGHTX},
    {"righty", ControllerButton::AXIS_RIGHTY},
    {"lefttrigger", ControllerButton::AXIS_TRIGGERLEFT},
    {"righttrigger", ControllerButton::AXIS_TRIGGERRIGHT},
};


std::string_view EventType::get_name() const
{
    const char *event_name = libevdev_event_code_get_name(type, code);
    if (event_name == nullptr) {
        event_name = "";
    }
    return event_name;
}


//////////////////////////////////////////////////////////////////////////
// SDLJoyMapping class
//////////////////////////////////////////////////////////////////////////

ControllerMapping::ControllerMapping(std::string const &mapping_str, priority_t priority)
{
    std::vector<std::string> elements = string::split(mapping_str, ",");
    
    this->priority = priority;
    if (elements.size() < 3) {
        LOG(ERROR) << "ControllerMapping: wrong format of mapping string\n" \
            << mapping_str;

        throw std::runtime_error("Wrong format of mapping string");
    }
    guid = elements[0];
    name = elements[1];
    for(int i=2; i<elements.size(); i++) {
        if (elements[i].length() > 0) {
            parse_config_element(elements[i]);
        }
    }
}

void ControllerMapping::parse_config_element(const std::string &item)
{
    std::vector<std::string> element = string::split(item, ":", 1);
    if (element.size() != 2) { 
        LOG(ERROR) << "Wrong option value: " << item << std::endl;
        return;
    }
    std::string key = element[0];
    std::string value = element[1];

    if (key == "platform") {
        platform = value;
    } else if (key == "hint") {
        hints[key] = value;
    } else {
        if (value.length() > 0) {
            add_button_binding(key, value);
        }
    }
    
}

ControllerButton ControllerMapping::get_button_from_string(const std::string &name)
{
    auto it = StringToControllerButton.find(string::strip(name, "+-"));
    if (it == StringToControllerButton.end()) {
        return ControllerButton::INVALID;
    } else {
        return it->second;
    }
}

void ControllerMapping::add_button_binding(const std::string &button_name, const std::string &button_def)
{
    ButtonBinding binding;
    ControllerButton button = get_button_from_string(button_name);

    if (button == ControllerButton::INVALID) {
        LOG(ERROR) << "add_button_binding:" << "invalid button_name: " << button_name;
        return;
    }

    if ( (button > ControllerButton::BUTTON_MAX) && 
        (button < ControllerButton::AXIS_MAX) ) 
    {
        binding.output_type = BindType::BINDTYPE_AXIS;
        binding.output.axis = button;
        if ((button == ControllerButton::AXIS_TRIGGERLEFT) || (button == ControllerButton::AXIS_TRIGGERRIGHT)) {
            binding.output.axis_type = ControllerAxisType::HALF_AXIS_POSITIVE;
        } if (button_name[0] == '+') {
            binding.output.axis_type = ControllerAxisType::HALF_AXIS_POSITIVE;
        } if (button_name[0] == '-') {
            binding.output.axis_type = ControllerAxisType::HALF_AXIS_NEGATIVE;
        } else {
            binding.output.axis_type = ControllerAxisType::FULL_AXIS;
        }
    } else if (button < ControllerButton::BUTTON_MAX) {
        binding.output_type = BindType::BINDTYPE_BUTTON;
        binding.output.button = button;
    } else {
        LOG(ERROR) << "add_button_binding: unsupported button type ControllerButton(" << static_cast<int>(button) << ")";
        return; // Unsupported button type
    }

    try {
        int i_button_def = 0;
        ControllerAxisType axis_input_type = ControllerAxisType::FULL_AXIS;
        if (button_def.at(i_button_def) == '+') {
            axis_input_type = ControllerAxisType::HALF_AXIS_POSITIVE;
            ++i_button_def;
        } else if (button_def.at(i_button_def) == '-') {
            axis_input_type = ControllerAxisType::HALF_AXIS_NEGATIVE;
            ++i_button_def;
        }

        if (button_def.at(i_button_def) == 'a') {
            i_button_def++;
            binding.input_type = BindType::BINDTYPE_AXIS;
            binding.input.axis = std::stoi( button_def.substr(i_button_def++, 1) );
            binding.input.axis_type = axis_input_type;
            binding.input.invert_input = button_def.at(button_def.length()-1) == '~';
        } else if (button_def.at(i_button_def) == 'b') {
            i_button_def++;
            binding.input_type = BindType::BINDTYPE_BUTTON;
            binding.input.button = std::stoi( button_def.substr(i_button_def++, 1) );
        } else if ( (button_def.at(i_button_def) == 'h') && (button_def.at(i_button_def+2) == '.') )  {
            i_button_def++;
            binding.input_type = BindType::BINDTYPE_HAT;
            binding.input.hat = std::stoi( button_def.substr(i_button_def, 1) );
            binding.input.hat_mask = std::stoi( button_def.substr(i_button_def+2, 1) );
            i_button_def+=2;
        } else {
            LOG(ERROR) << "add_button_binding: unexpected joystick element \"" \
                <<  button_def.at(i_button_def) << "\""; 
            return; // Unexpected joystick element 
        }
    } catch (const std::out_of_range& oor) {
        LOG(ERROR) << "add_button_binding: unexpected end of the button definition\n" \
            << button_def; 
        return; //Error
    } catch (const std::invalid_argument& ia) {
        LOG(ERROR) << "add_button_binding: invalid format of the button\n" \
            << button_def; 
        return; //Error
    }
    button_binding.emplace(button, binding);
}

//////////////////////////////////////////////////////////////////////////
// SDLJoyMapping class
//////////////////////////////////////////////////////////////////////////

SDLJoyMapping::SDLJoyMapping()
{
}


void SDLJoyMapping::add_sdl_mapping(ControllerMapping const &mapping)
{
    if ((mapping.platform != "") && (mapping.platform != get_platform())) {
        LOG(INFO) << "add_sdl_mapping: skip to add mapping for platform: " << mapping.platform;
        return;
    }
    
    auto search = mapping_db.find(mapping.guid);
    if (search != mapping_db.end()) {
        if (mapping.priority >= search->second.priority) {
            mapping_db[mapping.guid] = mapping;
        }
    } else {
        mapping_db[mapping.guid] = mapping;
    }
}

void SDLJoyMapping::add_sdl_mapping(std::istream &iss, 
        ControllerMapping::priority_t priority)
{
    std::string line;
    while (std::getline(iss, line)) {
        ControllerMapping mapping(line, priority);
        add_sdl_mapping(mapping);
    }
}

void SDLJoyMapping::add_user_mappings()
{
    const char *env_var_value = std::getenv("SDL_GAMECONTROLLERCONFIG");
    if (env_var_value != nullptr) {
        std::istringstream iss(env_var_value);
        add_sdl_mapping(iss, ControllerMapping::PRIORITY_USER);
    }
}

void SDLJoyMapping::add_internal_mappings()
{
    // Search internal mapping database
    for (int i=0; s_ControllerMappings[i] != NULL; i++) {
        ControllerMapping mapping(s_ControllerMappings[i], ControllerMapping::PRIORITY_DEFAULT);
        add_sdl_mapping(mapping);
    }
}

ControllerMapping* SDLJoyMapping::get_mapping(std::string const &guid)
{
    auto it = mapping_db.find(guid);
    if ( it != mapping_db.end() ) {
        LOG(INFO) << "Found joystick mapping for guid: " << guid
            << ", " << it->second.name;
        return &it->second;
    }
    return nullptr;
}


SDLJoyMapping::~SDLJoyMapping()
{

}

//////////////////////////////////////////////////////////////////////////
// EvdevJoystick class
//////////////////////////////////////////////////////////////////////////
std::vector<std::string> EvdevJoystick::get_event_devices()
{
    std::vector<std::string> ev_devices;
    const std::string_view path = "/dev/input/by-path";

    std::string filename;

    for (auto const & entry : fs::directory_iterator(path)) {
        filename = entry.path().u8string();

        if (string::endswith(filename, std::string("event-joystick")) ) {
            ev_devices.push_back(filename);
        }
    }

    return ev_devices;
}

EvdevJoystick::EvdevJoystick(std::string const &devname)
{
    int fd = open(devname.c_str(), O_RDONLY|O_NONBLOCK);
    int rc = libevdev_new_from_fd(fd, &evdev);

    LOG(INFO) << "EvdevJoystick: open device: " << devname;
    if (rc < 0) {
        LOG(ERROR) << "Failed to init libevdev (" << devname << ")\n" \
            << "  " << std::strerror(-rc);
        throw std::runtime_error("Failed to init libevdev");
    }
    this->devname = devname;
    get_button_settings();
    get_hat_settings();
    get_axes_settings();
}

void EvdevJoystick::get_guid(joy_guid_t &guid)
{
    uint16_t *guid16 = reinterpret_cast<uint16_t*>(&guid);
    guid16[0] = to_int16le(libevdev_get_id_bustype(evdev));
    guid16[1] = 0;
    guid16[2] = to_int16le(libevdev_get_id_vendor(evdev));
    guid16[3] = 0;
    guid16[4] = to_int16le(libevdev_get_id_product(evdev));
    guid16[5] = 0;
    guid16[6] = to_int16le(libevdev_get_id_version(evdev));
    guid16[7] = 0;
}

std::string_view EvdevJoystick::get_name()
{
    return libevdev_get_name(evdev);
}

std::string EvdevJoystick::get_guid()
{
    static const char hex2ascii[] = "0123456789abcdef";

    joy_guid_t guid;
    get_guid(guid);
    std::string result;
    
    result.reserve(32);
    for (int i=0; i<sizeof(guid); i++) {
        result += hex2ascii[(guid[i] >> 4)];
        result += hex2ascii[(guid[i] & 0x0f )];
    }
    return result;
}

void EvdevJoystick::get_button_settings()
{
    int event_code;

    buttons.clear();
    for (event_code=BTN_JOYSTICK; event_code < KEY_MAX; ++event_code) {
        if (libevdev_has_event_code(evdev, EV_KEY, event_code)) {
            buttons.push_back(EventType(EV_KEY, event_code));
            LOG(DEBUG) << "get_button_settings: button: " \
                << buttons.size()-1 << ", " << buttons.back().get_name();
        }
    }
    for (event_code=BTN_MISC; event_code < BTN_JOYSTICK; ++event_code) {
        if (libevdev_has_event_code(evdev, EV_KEY, event_code)) {
            buttons.push_back(EventType(EV_KEY, event_code));
            LOG(DEBUG) << "get_button_settings: button:" \
                << buttons.size()-1 << ", " << buttons.back().get_name();
        }
    }
}

void EvdevJoystick::get_hat_settings()
{
    int event_code;

    hats.clear();
    for (event_code=ABS_HAT0X; event_code < ABS_HAT3Y; event_code += 2) {
        if (libevdev_has_event_code(evdev, EV_ABS, event_code) || 
            libevdev_has_event_code(evdev, EV_ABS, event_code+1)) 
        {
            const struct input_absinfo *absinfo = libevdev_get_abs_info(evdev, event_code);
            if (absinfo == nullptr) continue;
            hats.push_back( HatEventType{
                                EventType(EV_ABS, event_code),
                                EventType(EV_ABS, event_code+1)
                            }
            );
            LOG(DEBUG) << "get_hat_settings: hat: " << hats.size()-1 \
                << ", " << hats.back().x.get_name();
        }
    }
}

void EvdevJoystick::get_axes_settings()
{
    int event_code;

    axes.clear();
    for (event_code=0; event_code < ABS_MAX; event_code++) {
        // Skip hats
        if (event_code == ABS_HAT0X) {
            event_code = ABS_HAT3Y;
            continue;
        }
         if (libevdev_has_event_code(evdev, EV_ABS, event_code)) {
            const struct input_absinfo *absinfo = libevdev_get_abs_info(evdev, event_code);
            axes.push_back(EventType(EV_ABS, event_code));
            LOG(DEBUG) << "get_axes_settings: axes: " << axes.size()-1 \
                << ", " << axes.back().get_name();
         }
    }

}

void EvdevJoystick::set_mapping(ControllerMapping const &mapping)
{
    this->mapping = mapping;
}

bool EvdevJoystick::set_mapping(SDLJoyMapping &joy_mapping)
{
    ControllerMapping *pmapping=nullptr;

    if  (pmapping=joy_mapping.get_mapping(get_guid())) {
        mapping = *pmapping;
        return true;
    }
    return false;
}

EventButtonBinding EvdevJoystick::get_event_binding(ControllerButton button)
{
    EventButtonBinding event_binding;
    auto const it=mapping.button_binding.find(button);

    if (it != mapping.button_binding.end()) {
        event_binding.bind = &it->second;
        try {
            if (it->second.input_type == BindType::BINDTYPE_BUTTON) {
                event_binding.event = &buttons.at(it->second.input.button);
            } else 
            if (it->second.input_type == BindType::BINDTYPE_HAT) {
                int hat_mask = it->second.input.hat_mask;
                if ( hat_mask & (HatMask::UP | HatMask::DOWN) ) {
                    event_binding.event = &hats.at(it->second.input.hat).y;
                } else
                if ( hat_mask & (HatMask::LEFT | HatMask::RIGHT) ) {
                    event_binding.event = &hats.at(it->second.input.hat).x;
                } else {
                    LOG(ERROR) << "get_event_binding: Wrong value of HAT:" << it->second.input.hat
                        << " mask: " << hat_mask;
                    event_binding.bind = nullptr;
                    event_binding.event = nullptr;
                }
                
            } else 
            if (it->second.input_type == BindType::BINDTYPE_AXIS) {
                event_binding.event = &axes.at(it->second.input.axis);
            } else {
                LOG(ERROR) << "Unexpected BindType value: " << static_cast<int>(it->second.input_type);
            }
        } catch (std::out_of_range const& exc) {
            event_binding.bind = nullptr;
            event_binding.event = nullptr;
        }

    }
    return event_binding;
}

EvdevJoystick::~EvdevJoystick()
{
    if (evdev != nullptr) {
        int fd = libevdev_get_fd(evdev);
        if (fd != -1) {
            close(fd);
            LOG(DEBUG) << "~EvdevJoystick: close evdev file";
        }
        libevdev_free(evdev);
        LOG(DEBUG) << "~EvdevJoystick: libevdev_free";
        evdev = nullptr;
    }
}


} // namespace evdevjoy