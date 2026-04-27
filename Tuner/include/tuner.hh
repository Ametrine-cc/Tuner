#ifndef TUNER_HH
#define TUNER_HH

// LIBRARIES
#include "kamakazi"

// DEFINES
#define DEFAULT_CONFIG_FILE_PATH ".tuner/config"
#define MAX_BUFFER_SIZE 1024

#define HOME std::getenv("HOME")
#define TUNER_DIR "/usr/share/Tuner"

// FUNCTIONS
int checkConfig();

#endif // TUNER_HH
