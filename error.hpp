
#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

// show an error
#define ERROR(x) do { std::cout << "ERROR: " << x << std::endl; } while (0)
// show a non-fatal warning
#define WARN(x) do { std::cout << "WARN: " << x << std::endl; } while (0)
// show a standard message
#define MSG(x) do { std::cout << "MSG: " << x << std::endl; } while (0)
// produce a message but with no newline
#define MSGNONEWLINE(x) do { std::cout << x; } while (0)

#endif
