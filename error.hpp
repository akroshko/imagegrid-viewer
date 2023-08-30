/**
 * Give errors and other messages for the user, seperate from debugging information.
 */
#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

// show an error
#define ERROR(x) do { std::cerr << "ERROR: " << x << std::endl; } while (0)
// show a non-fatal warning
#define WARN(x) do { std::cerr << "WARN: " << x << std::endl; } while (0)
// show a standard message
#define MSG(x) do { std::cerr << "MSG: " << x << std::endl; } while (0)
// produce a message but with no newline
#define MSGNONEWLINE(x) do { std::cerr << x; } while (0)

#endif
