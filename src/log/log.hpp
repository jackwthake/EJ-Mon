#ifndef LOG_HPP
#define LOG_HPP

#define WAIT_FOR_SERIAL 1
#define LOGGING_ENABLED 1

#if LOGGING_ENABLED == 0

#define LOG_BEGIN() 
#define LOG_PRINTLN(fmt, ...) 
#define LOG_PRINT(fmt, ...) 
#define LOG_ERROR(fmt, ...) 
#define LOG_PANIC(fmt, ...) 

#else

#define LOG_BEGIN() \
Serial.begin(115200); \
if (WAIT_FOR_SERIAL) { \
    while (!Serial) { ; } \
}

#define LOG_PRINTLN(fmt, ...) \
do { \
    if (WAIT_FOR_SERIAL ) { \
        while (!Serial) { ; } \
    } \
    Serial.printf(fmt "\r\n", ##__VA_ARGS__); \
} while (0);\

#define LOG_PRINT(fmt, ...) \
do { \
    if (WAIT_FOR_SERIAL) { \
        while (!Serial) { ; } \
    } \
    Serial.printf(fmt, ##__VA_ARGS__); \
} while (0); \

#define LOG_ERROR(fmt, ...) \
do { \
    if (WAIT_FOR_SERIAL) { \
        while (!Serial) { ; } \
    } \
    Serial.printf("ERROR: " fmt "\r\n", ##__VA_ARGS__); \
} while (0);

#define LOG_PANIC(fmt, ...) \
do { \
    if (WAIT_FOR_SERIAL) { \
        while (!Serial) { ; } \
    } \
    Serial.printf("PANIC: " fmt "\r\n", ##__VA_ARGS__); \
    Serial.flush(); \
    while (1) { ; } \
} while (0);

#endif // LOGGING_ENABLED

#endif // LOG_HPP