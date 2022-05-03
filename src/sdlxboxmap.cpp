#include <iostream>
#include <filesystem>
#include <string>
#include <cstring>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <chrono>
#include <fstream>
#include <ios>

#include "logging.h"
#include "sdlxboxmap.h"
#include "stringext.h"
#include "bitext.h"
#include "platform.h"

using namespace evdevjoy;
using std::chrono::high_resolution_clock;

namespace fs = std::filesystem;
// https://gist.github.com/meghprkh/9cdce0cd4e0f41ce93413b250a207a55
//https://meghprkh.github.io/blog/posts/handling-joysticks-and-gamepads-in-linux/


void test_joystick(std::string const &dev_path) {
    int rc;
    int fd;
    struct libevdev *dev = NULL;

    fd = open(dev_path.c_str(), O_RDONLY|O_NONBLOCK);
    rc = libevdev_new_from_fd(fd, &dev);
    std::cout << "Open device: " << dev << std::endl;
    if (rc < 0) {
        std::cerr << "Failed to init libevdev (" << dev_path << ")\n" \
            << std::strerror(-rc) << std::endl;
        exit(1);
    }
    
    std::cout << "Device name: " << libevdev_get_name(dev) << std::endl;
    do {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0) {
            std::cout << "EVENT type: " << ev.type << ", code: " << ev.code << ", value" << ev.value << std::endl;
        }
    } while (rc == 1 || rc == 0 || rc == -EAGAIN);
}

namespace sdlxboxmap {
namespace ev = evdevjoy;

typedef std::unordered_map<std::string_view, ControllerButton> t_MapButton2SDLButton;
static const t_MapButton2SDLButton MapButton2SDLButton = {
    {"a", ControllerButton::BUTTON_A},
    {"b", ControllerButton::BUTTON_B},
    {"x", ControllerButton::BUTTON_X},
    {"y", ControllerButton::BUTTON_Y},
    {"back", ControllerButton::BUTTON_BACK},
    {"guide", ControllerButton::BUTTON_GUIDE},
    {"start", ControllerButton::BUTTON_START},
    {"tl", ControllerButton::BUTTON_LEFTSTICK},
    {"leftstick", ControllerButton::BUTTON_LEFTSTICK},
    {"tr", ControllerButton::BUTTON_LEFTSTICK},
    {"rightstick", ControllerButton::BUTTON_RIGHTSTICK},
    {"lb", ControllerButton::BUTTON_LEFTSHOULDER},
    {"leftshoulder", ControllerButton::BUTTON_LEFTSHOULDER},
    {"rb", ControllerButton::BUTTON_RIGHTSHOULDER},
    {"rightshoulder", ControllerButton::BUTTON_RIGHTSHOULDER},
    {"dpad_y", ControllerButton::BUTTON_DPAD_UP},
    {"dpup", ControllerButton::BUTTON_DPAD_UP},
    {"duup", ControllerButton::BUTTON_DPAD_UP},
    {"du", ControllerButton::BUTTON_DPAD_UP},
    {"dpdown", ControllerButton::BUTTON_DPAD_DOWN},
    {"dd", ControllerButton::BUTTON_DPAD_DOWN},
    {"ddown", ControllerButton::BUTTON_DPAD_DOWN},
    {"dpad_x", ControllerButton::BUTTON_DPAD_LEFT},
    {"dpleft", ControllerButton::BUTTON_DPAD_LEFT},
    {"dlleft", ControllerButton::BUTTON_DPAD_LEFT},
    {"dl", ControllerButton::BUTTON_DPAD_LEFT},
    {"dpright", ControllerButton::BUTTON_DPAD_RIGHT},
    {"drright", ControllerButton::BUTTON_DPAD_RIGHT},
    {"dr", ControllerButton::BUTTON_DPAD_RIGHT},
    {"misc1", ControllerButton::BUTTON_MISC1},
    {"paddle1", ControllerButton::BUTTON_PADDLE1},
    {"paddle2", ControllerButton::BUTTON_PADDLE2},
    {"paddle3", ControllerButton::BUTTON_PADDLE3},
    {"paddle4", ControllerButton::BUTTON_PADDLE4},
    {"touchpad", ControllerButton::BUTTON_TOUCHPAD},
    {"x1", ControllerButton::AXIS_LEFTX},
    {"leftx", ControllerButton::AXIS_LEFTX},
    {"y1", ControllerButton::AXIS_LEFTY},
    {"lefty", ControllerButton::AXIS_LEFTY},
    {"x2", ControllerButton::AXIS_RIGHTX},
    {"rightx", ControllerButton::AXIS_RIGHTX},
    {"y2", ControllerButton::AXIS_RIGHTY},
    {"righty", ControllerButton::AXIS_RIGHTY},
    {"lt", ControllerButton::AXIS_TRIGGERLEFT},
    {"lefttrigger", ControllerButton::AXIS_TRIGGERLEFT},
    {"rt", ControllerButton::AXIS_TRIGGERRIGHT},
    {"righttrigger", ControllerButton::AXIS_TRIGGERRIGHT},
};

MainAppException::MainAppException(const std::string& msg) :
    std::runtime_error(msg)
{
    LOG(ERROR) << "MainAppException: " << msg;
}

MainApp::MainApp()
{
    joymap.add_default_mapping();
}

std::unique_ptr<cxxopts::Options> MainApp::init_arg_parser()
{
    auto options = std::make_unique<cxxopts::Options>(
        "sdlxboxmap",
        "Command line tool to generate xboxdrv configuation file based on the\n"
        "configuration templates. Gamepad button mapping is made with help of SDL\n"
        "database."
    );
    options->show_positional_help();
    options->add_options()
        ("h,help", "Print this help", cxxopts::value<bool>()->default_value("false"))
        ("l,list", "List of gamepad devices")
        ("t,template", "Configuration template", cxxopts::value<std::string>())
        ("d,tdir", "Directory (database) with list of template files based on "
            "<guid_id>.tpl", cxxopts::value<std::string>())
        ("g,guid", "For which gamepad guid prepare the configuration", 
            cxxopts::value<std::vector<std::string>>()
            ->default_value(""))
        ("f,filter-guid", "Filter out given gamepad guid ids", 
            cxxopts::value<std::vector<std::string>>()
            ->default_value(""))
        ("o,output", "Output file", cxxopts::value<std::string>())
        ("log", "Logfile, disabled by default", cxxopts::value<std::string>())
        ;
    return options;
}

void MainApp::arg_parse(int argc, char* argv[])
{
    auto options = init_arg_parser();
    try {
        cxxopts::ParseResult parsed_args = options->parse(argc, argv);
        if (parsed_args.count("log")) {
            logging_to_file(parsed_args["log"].as<std::string>());
        }
        if (parsed_args.count("list")) {
            find_gamepads();
        } else if (parsed_args.count("output")) {
            make_file_substitution(parsed_args);
        } else {
            std::cout << options->help() << std::endl;
            exit(1);
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
    catch (const MainAppException& e) 
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }
}

void MainApp::make_file_substitution(cxxopts::ParseResult &parsed_args)
{
    std::vector<t_uptr_evdevjoystick> gamepads;
    fs::path arg_tpl_path;
    fs::path arg_tpl_dir;
    std::vector<std::string> guid_list;
    std::vector<std::string> guid_filter;

    if (parsed_args["guid"].count()) {
        guid_list = parsed_args["filter-guid"].as<std::vector<std::string>>();
    }

    if (parsed_args["filter-guid"].count()) {
        guid_filter = parsed_args["filter-guid"].as<std::vector<std::string>>();
    }

    init_gamepads(guid_list, guid_filter, gamepads);

    if (gamepads.size() == 0) {
        throw MainAppException("There is not connected any gamepad.");
    }

    arg_tpl_path = parsed_args["template"].as<std::string>();
    if (!fs::exists(arg_tpl_path)) {
        throw MainAppException("Template file '" + arg_tpl_path.string() + "' does not exist.");
    }

    if (parsed_args.count("tdir")) {
        arg_tpl_dir = parsed_args["tdir"].as<std::string>();
        
        if (fs::is_directory(arg_tpl_dir)) {
            LOG(INFO) << "Specified template directory: " << arg_tpl_dir.string() << "\n"
                << "  file will be searched in file: <guid_id>.tpl";
        } else {
            throw MainAppException("Template directory: '" + arg_tpl_dir.string() + "'does not exist");
        }
    }

    std::string arg_out_filename = parsed_args["output"].as<std::string>();
    std::vector<std::string> voutput = string::rsplit(arg_out_filename, ".", 1);

    std::string file_name_base = voutput[0];
    std::string file_ext = "";

    if (voutput.size() > 1) {
        file_ext = voutput[1];
    }

    fs::path tpl_filename;
    for(int i = 0; i < gamepads.size(); i++) {
        if (i>0) {
            arg_out_filename = file_name_base + std::to_string(i) + "." + file_ext;
        }

        if (!arg_tpl_dir.empty()) {
            std::string _filename_tpl = gamepads[i]->get_guid() + ".tpl";
            tpl_filename = arg_tpl_dir / _filename_tpl;
            if (!fs::exists(tpl_filename)) {
                LOG(INFO) << "Specific template does not exist: " << tpl_filename.string();
                tpl_filename = arg_tpl_path;
            }
        } else {
            tpl_filename = arg_tpl_path;
        }
        replace_mapping(*gamepads[i], tpl_filename, arg_out_filename);
    }
}

void MainApp::init_gamepads(
        const std::vector<std::string> &guid,
        const std::vector<std::string> &filter,
        std::vector<t_uptr_evdevjoystick> &gamepads)
{
    std::vector<std::string> ev_devices = EvdevJoystick::get_event_devices();
    std::string guid_it;
    bool match_guid;

    for(auto path : ev_devices) {
        auto tmp_gamepad = std::make_unique<evdevjoy::EvdevJoystick>(path);
        guid_it = tmp_gamepad->get_guid();

        match_guid = true;
        if (guid.size() > 0) {
            if (std::find(guid.begin(), guid.end(), guid_it) == guid.end()) {
                LOG(WARNING) << "Skip gamepad, not in the list (option --guid): " << guid_it;
                match_guid = false;
            }
        }

        if (std::find(filter.begin(), filter.end(), guid_it) != filter.end()) {
            match_guid = false;
            LOG(WARNING) << "Filter out gamepad (option --filter-guid): " << guid_it;
        }

        if (match_guid) {
            tmp_gamepad->set_mapping(joymap);
            gamepads.push_back(std::move(tmp_gamepad));
        }
    }
}

void MainApp::find_gamepads()
{
    std::vector<std::string> ev_devices = EvdevJoystick::get_event_devices();

    for(auto path : ev_devices) {
        EvdevJoystick joy(path);
        std::cout << joy.get_guid()
            << "\t" << joy.get_name() 
            << "\t" << path
            << std::endl;
    }
}

void MainApp::replace_mapping(evdevjoy::EvdevJoystick &gamepad, const std::string &tpl_filename, 
        const std::string &out_filename)
{
    LOG(INFO) << "replace_mapping\n"
        << "  template: " << tpl_filename << "\n"
        << "  output file: " << out_filename;

    std::ifstream fin;
    std::ofstream fout;

    fin.exceptions(std::ifstream::badbit);
    fout.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fin.open(tpl_filename, std::ios::in);
        fout.open(out_filename, std::ios::out | std::ios::trunc);
        sreplace_mapping(gamepad, fin, fout);
    }
    catch (const std::ios_base::failure& e) {
        LOG(ERROR) << "Error during opening file (errorcode: " << e.code()  << ")\n"
            << e.what() << std::endl;
    }


}

void MainApp::sreplace_mapping(evdevjoy::EvdevJoystick &gamepad, std::istream &is, std::ostream &os)
{
    std::string line;
    std::string result_line;
    std::string::size_type i_cmd_start, i_cmd_end, i_start;
    std::string command;
    std::string mapped_value;
    bool unsupported;
    e_mapping_result mapping_result;

    while (std::getline(is, line)) {
        result_line.clear();
        i_cmd_start = 0;
        i_start = 0;
        unsupported = false;

        while ((i_cmd_start=line.find("<", i_start)) != std::string::npos)
        {
            i_cmd_end = line.find(">", i_cmd_start + 1);
            if (i_cmd_end != std::string::npos) {
                command = line.substr(i_cmd_start+1, i_cmd_end-i_cmd_start-1);
                result_line.append(line, i_start, i_cmd_start);
                
                mapping_result = get_mapping_value(gamepad, command, mapped_value);
                if (mapping_result == e_mapping_result::FOUND) {
                    result_line.append(mapped_value);
                } else if (mapping_result == e_mapping_result::NOT_FOUND) {
                    result_line.append(line, i_cmd_start, i_cmd_end-i_cmd_start+1);
                } else if (mapping_result == e_mapping_result::UNSUPPORTED) {
                    result_line.append(mapped_value);
                    unsupported = true;
                }
                i_start = i_cmd_end + 1;
            } else {
                result_line.append(line, i_start, std::string::npos);
                break;
            }
        }

        result_line.append(line, i_start, std::string::npos);
        if (unsupported) {
            if (result_line.size() > 0) {
                os << "# " << result_line << "\n";
            }
        } else {
            os << result_line << "\n";
        }

    }
}
#if 0
void MainApp::sreplace_mapping(evdevjoy::EvdevJoystick &gamepad, std::istream &is, std::ostream &os)
{
    std::streambuf *osbuf = os.rdbuf();
    char buffer[BUFFER_SIZE];
    e_mapping_result mapping_result;
    std::string mapped_value;

    while (is.get(*osbuf, '<').good()) {
        mapping_result = false;
        is.get(buffer, BUFFER_SIZE, '>');

        if (is.gcount() < BUFFER_SIZE) {
            mapping_result = get_mapping_value(
                gamepad,
                std::string_view(buffer+1, is.gcount()-1),
                mapped_value);

            if (mapping_result) {
                os << mapped_value;
                is.ignore(1); // Skip right angle bracket
            }
        } 
        
        if (!mapping_result && (is.gcount() > 0))  { 
            os.write(buffer, is.gcount());
        }
        std::cout << "loop: " << buffer << "count = "<< is.gcount() << std::endl;
    }

}

#endif

MainApp::e_mapping_result MainApp::map_evdev(evdevjoy::EvdevJoystick &gamepad, std::string &result)
{
    result = gamepad.devname;
    return e_mapping_result::FOUND;
}

MainApp::e_mapping_result MainApp::map_abs(evdevjoy::EvdevJoystick &gamepad, 
        std::string_view button_name, std::string &result)
{
    std::string button_name_lower = string::lower(button_name);
    t_MapButton2SDLButton::const_iterator it = MapButton2SDLButton.find(button_name_lower);
    e_mapping_result retvalue = e_mapping_result::UNSUPPORTED;
    
    if (it != MapButton2SDLButton.end()) {
        EventButtonBinding event_binding = gamepad.get_event_binding(it->second);
        if (event_binding) {
            if ( (event_binding.bind->input_type == BindType::BINDTYPE_AXIS) || 
                 (event_binding.bind->input_type == BindType::BINDTYPE_HAT)) {
                result = event_binding.event->get_name();
                retvalue = e_mapping_result::FOUND;
            } else {
                LOG(WARNING) << "MAP_BUTTON: Error to map: " << button_name << "\n"
                    << "  button must be of the type axis or hat only";
                result = "ERROR ";
                result.append(event_binding.event->get_name());
                result.append(" is button"); 
                retvalue = e_mapping_result::UNSUPPORTED;
            }
        } else {
            LOG(ERROR) << "Unsupported gamepad button: " << button_name;
            result = "Unsupported gamepad mapping: ";
            result.append(button_name);  
            retvalue = e_mapping_result::UNSUPPORTED;
        }
    } else {
        LOG(ERROR) << "Cannot find button mapping for: " << button_name;
        result = "Unknown button: ";
        result.append(button_name);
        retvalue = e_mapping_result::UNSUPPORTED;
    }

    return retvalue;
}


MainApp::e_mapping_result MainApp::map_button(evdevjoy::EvdevJoystick &gamepad, 
        std::string_view button_name, std::string &result)
{
    std::string button_name_lower = string::lower(button_name);
    t_MapButton2SDLButton::const_iterator it = MapButton2SDLButton.find(button_name_lower);
    e_mapping_result retvalue = e_mapping_result::UNSUPPORTED;
    
    result.clear();
    if (it != MapButton2SDLButton.end()) {
        EventButtonBinding event_binding = gamepad.get_event_binding(it->second);
        if (event_binding) {
            if (event_binding.bind->input_type == BindType::BINDTYPE_BUTTON) {
                result = event_binding.event->get_name();
                retvalue = e_mapping_result::FOUND;
            } else {
                LOG(WARNING) << "MAP_BUTTON: Error to map: " << button_name << "\n"
                    << "  button must be of the type button only (not hat, or axes)";
                result = "ERROR ";
                result.append(event_binding.event->get_name());
                result.append(" is axes"); 
                retvalue = e_mapping_result::UNSUPPORTED;
            } 
        } else {
            LOG(ERROR) << "Unsupported gamepad button: " << button_name;
            result = "Unsupported gamepad mapping: ";
            result.append(button_name);  
            retvalue = e_mapping_result::UNSUPPORTED;
        }
    } else {
        LOG(ERROR) << "Cannot find button mapping for: " << button_name;
        result = "Unknown button: ";
        result.append(button_name);
        retvalue = e_mapping_result::UNSUPPORTED;
    }

    return retvalue;
}

MainApp::e_mapping_result MainApp::axismap(evdevjoy::EvdevJoystick &gamepad,
        std::string_view button_name, std::string &result)
{
    e_mapping_result retvalue = e_mapping_result::UNSUPPORTED;

    if (button_name.size() < 2) {
        result = "Wrong axis name: ";
        result.append(button_name);
        return e_mapping_result::UNSUPPORTED;
    }

    bool invert_axis = false;
    
    if (button_name[0] == '+') {
        button_name.remove_prefix(1);
    } else if (button_name[0] == '-') {
        invert_axis = !invert_axis;
        button_name.remove_prefix(1);
    }
    
    std::string button_name_lower = string::lower(button_name);
    t_MapButton2SDLButton::const_iterator it = MapButton2SDLButton.find(button_name_lower);
    if (it != MapButton2SDLButton.end()) {
        EventButtonBinding event_binding = gamepad.get_event_binding(it->second);
        if (event_binding) {
            if (event_binding.bind->input_type == BindType::BINDTYPE_AXIS) {
                if (event_binding.bind->output_type == BindType::BINDTYPE_AXIS) {
                    if (event_binding.bind->output.axis_type == ControllerAxisType::HALF_AXIS_NEGATIVE) {
                        invert_axis = !invert_axis;
                    }
                }
                if (event_binding.bind->input.invert_input) {
                    invert_axis = !invert_axis;
                }
                retvalue = e_mapping_result::FOUND;
            } else if ((event_binding.bind->input_type == BindType::BINDTYPE_HAT) && 
                    (event_binding.bind->output_type == BindType::BINDTYPE_BUTTON)) {
                if ( (event_binding.bind->output.button == ControllerButton::BUTTON_DPAD_LEFT) && 
                    (event_binding.bind->input.hat_mask == HatMask::RIGHT)) {
                        invert_axis = !invert_axis;
                } else if ( (event_binding.bind->output.button == ControllerButton::BUTTON_DPAD_RIGHT) && 
                    (event_binding.bind->input.hat_mask == HatMask::LEFT)) {
                        invert_axis = !invert_axis;
                } else if ( (event_binding.bind->output.button == ControllerButton::BUTTON_DPAD_UP) && 
                    (event_binding.bind->input.hat_mask == HatMask::DOWN)) {
                        invert_axis = !invert_axis;
                } else if ( (event_binding.bind->output.button == ControllerButton::BUTTON_DPAD_DOWN) && 
                    (event_binding.bind->input.hat_mask == HatMask::UP)) {
                        invert_axis = !invert_axis;
                }
                retvalue = e_mapping_result::FOUND;
            }
            
            if (retvalue == e_mapping_result::FOUND) {
                if (invert_axis) {
                    result.clear();
                    result.append("-");
                    result.append(button_name);
                } else {
                    result = button_name;
                }
            } else {
                LOG(ERROR) << "MAPAXIS: Unsupported mapping for: " << button_name << "\n"
                    << "Only axes or hats can be mapped.";
                result = button_name;
            }

        } else {
            LOG(ERROR) << "Unsupported gamepad button: " << button_name;
            result = "Unsupported gamepad mapping: ";
            result.append(button_name);  
            retvalue = e_mapping_result::UNSUPPORTED;
        }
    } else {
        LOG(ERROR) << "Cannot find button mapping for: " << button_name;
        result = "Unknown button: ";
        result.append(button_name);
        retvalue = e_mapping_result::UNSUPPORTED;
    }
    return retvalue;
}

MainApp::e_mapping_result MainApp::get_mapping_value(evdevjoy::EvdevJoystick &gamepad, 
    std::string_view map_command, std::string &result)
{
    std::vector<std::string_view> cmd_args = string::split(
        map_command, std::string(":"), 1);
    
    result.clear();

    if (cmd_args.size() == 1) {
        if (cmd_args[0] == "MAP_EVDEV") {
            return map_evdev(gamepad, result);
        }
    } else if (cmd_args.size() == 2) {
        if (cmd_args[0] == "MAP_ABS") {
            return map_abs(gamepad, cmd_args[1], result);
        } else if (cmd_args[0] == "MAP_BUTTON") {
            return map_button(gamepad, cmd_args[1], result);
        } else if (cmd_args[0] == "AXISMAP") {
            return axismap(gamepad, cmd_args[1], result);
        } 
    } 

    return e_mapping_result::NOT_FOUND;
}

} // namespace sdlxboxmap


class Argv {
  public:

  Argv(std::initializer_list<const char*> args)
  : m_argv(new char*[args.size()])
  , m_argc(args.size())
  {
    int i = 0;
    auto iter = args.begin();
    while (iter != args.end()) {
      auto len = strlen(*iter) + 1;
      auto ptr = std::unique_ptr<char[]>(new char[len]);

      strcpy(ptr.get(), *iter);
      m_args.push_back(std::move(ptr));
      m_argv.get()[i] = m_args.back().get();

      ++iter;
      ++i;
    }
  }

  char** argv() const {
    return m_argv.get();
  }

  int argc() const {
    return m_argc;
  }

  private:

  std::vector<std::unique_ptr<char[]>> m_args;
  std::unique_ptr<char*[]> m_argv;
  int m_argc;
};

int main(int argc, char* argv[]) {
    logging_init();

#if 0
    Argv dbg_args({"sdlxboxmap",
        "--default-log-file=test.log",
        "-t", "/home/martin/Unibox/cpp/sdlxboxmap/conf/Logitech_RumblePad.tpl",
        "-d", "/home/martin/Unibox/cpp/sdlxboxmap/conf/gamepad_db",
        "-o", "/home/martin/Unibox/cpp/sdlxboxmap/conf/Logitech_RumblePad.conf"});

    argc = dbg_args.argc();
    argv = dbg_args.argv();
#endif

    START_EASYLOGGINGPP(argc, argv);
    sdlxboxmap::MainApp app = sdlxboxmap::MainApp();

    app.arg_parse(argc, argv);
    return 0;
}
