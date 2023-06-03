
#ifndef DEBUG_HPP
#define DEBUG_HPP

// turn on/off debug messages
// these show progress of the application
#ifdef DEBUG_MESSAGES
#define DEBUG(x) do { std::cerr << "DEBUG: " << x << "\n" << std::flush; } while (0)
#else
#define DEBUG(x)
#endif

// turn on/off debug I/O messages
// show I/O, this tends to be much higher volume than standard DEBUG messages
#ifdef DEBUG_IO
#define DEBUGIO(x) do { std::cerr << "DEBUG IO: " << x << "\n" << std::flush; } while (0)
#else
#define DEBUGIO(x)
#endif

#endif
