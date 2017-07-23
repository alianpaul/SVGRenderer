#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>


#define out_msg(s) std::cout << "[MSG:  ] " << s << std::endl << std::flush
#define out_wrn(s) std::cout << "[WARN: ] " << s << std::endl << std::flush
#define out_err(s) std::cout << "[ERROR:] " << s << std::endl << std::flush


#endif