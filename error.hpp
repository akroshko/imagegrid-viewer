
#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

// show an error
#define ERROR(x) do { cout << "ERROR: " << x << endl; } while (0)
// show a non-fatal warning
#define WARN(x) do { cout << "WARN: " << x << endl; } while (0)
// show a standard message
#define MSG(x) do { cout << "MSG: " << x << endl; } while (0)
// produce a message but with no newline
#define MSGNONEWLINE(x) do { cout << x; } while (0)

#endif
