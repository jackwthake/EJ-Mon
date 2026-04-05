#ifndef LOG_HPP
#define LOG_HPP

#define LOGGING_ENABLED 1

#if LOGGING_ENABLED == 0

#define LOG_BEGIN() 
#define LOG_PRINTLN(fmt, ...) 
#define LOG_PRINT(fmt, ...) 
#define LOG_ERROR(fmt, ...) 
#define LOG_PANIC(fmt, ...) 

#else

// Platform-specific logging implementation
#ifdef PLATFORM_ESP32
  #define WAIT_FOR_SERIAL 1

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

#else // PLATFORM_DESKTOP

  #include <cstdio>

  #define LOG_BEGIN()

  #define LOG_PRINTLN(fmt, ...) \
  do { \
      printf(fmt "\n", ##__VA_ARGS__); \
      fflush(stdout); \
  } while (0);\

  #define LOG_PRINT(fmt, ...) \
  do { \
      printf(fmt, ##__VA_ARGS__); \
      fflush(stdout); \
  } while (0); \

  #define LOG_ERROR(fmt, ...) \
  do { \
      printf("ERROR: " fmt "\n", ##__VA_ARGS__); \
      fflush(stdout); \
  } while (0);

  #define LOG_PANIC(fmt, ...) \
  do { \
      printf("PANIC: " fmt "\n", ##__VA_ARGS__); \
      fflush(stdout); \
      exit(1); \
  } while (0);

#endif // PLATFORM_*

#endif // LOGGING_ENABLED

#endif // LOG_HPP