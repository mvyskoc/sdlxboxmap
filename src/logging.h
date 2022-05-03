#ifndef __LOGGING_H_INCLUDED
#define __LOGGING_H_INCLUDED

#include "easylogging++.h"

extern void logging_init();
extern void logging_to_file(const std::string &logfile);

#endif
