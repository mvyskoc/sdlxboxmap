#include "logging.h"

INITIALIZE_EASYLOGGINGPP


void logging_init() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    //defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    defaultConf.setGlobally(
            el::ConfigurationType::ToStandardOutput, "false");

    defaultConf.set(el::Level::Info,
            el::ConfigurationType::Format, "%datetime %level: %msg");
    
    defaultConf.set(el::Level::Debug,
            el::ConfigurationType::Format, "%datetime %level: %file, %msg");

    defaultConf.set(el::Level::Warning,
            el::ConfigurationType::Format, "%datetime %level: %file, %msg");

    defaultConf.set(el::Level::Error, 
        el::ConfigurationType::ToStandardOutput, "true");

    defaultConf.set(el::Level::Warning, 
        el::ConfigurationType::ToStandardOutput, "true");

/*
    defaultConf.setGlobally(
            el::ConfigurationType::ToFile, "true");
    defaultConf.setGlobally(
            el::ConfigurationType::Filename, "test.log");
*/
    el::Loggers::reconfigureLogger("default", defaultConf);
    el::Loggers::setDefaultConfigurations(defaultConf, true);

    // Register loggers
    el::Loggers::getLogger("evdevjoy");
    el::Loggers::getLogger("sdlxboxmap");
}

void logging_to_file(const std::string &logfile)
{
    el::Configurations c;
    
    c.setGlobally(el::ConfigurationType::Filename, logfile);
    el::Loggers::setDefaultConfigurations(c, true);
}