#ifndef FELDBUS_CONFIG_CHECK_AVR_H_
#define FELDBUS_CONFIG_CHECK_AVR_H_


#include <feldbus_config.h>
#ifdef __cplusplus
# include <cstdint>
#else
# include <stdint.h>
#endif

// hide some uninteresting stuff from documentation
#if (!defined(__DOXYGEN__))
    

#ifndef TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH
# error TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH must be defined
#else
# if (TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH != 1 ) && (TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH != 2)
#  error TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH must be 1 or 2
# endif
#endif

#ifndef TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE
# error TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE must be defined
#else
# if TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_16_MIXED
#  define TURAG_FELDBUS_DEVICE_CRC_SIZE 2
# else
#  define TURAG_FELDBUS_DEVICE_CRC_SIZE 1
# endif
#endif

#ifndef TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE
# error TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE must be defined
#endif

#ifndef TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
# error TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED must be defined
#else
# if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
#  warning TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED = 1
# endif
#endif

#ifndef TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY
# error TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY must be defined
#else
# if (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY<0) || (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY>65535)
#  error TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY must be within the range of 0-65535
# endif
#endif


#if TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE > 65535
# error buffer sizes greater than 65535 are no longer supported.
#elif TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE > 255
    typedef uint16_t FeldbusSize_t;
# if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 1
#  define TURAG_FELDBUS_NO_ANSWER 0xffff
# elif TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
#  define TURAG_FELDBUS_NO_ANSWER 0xfffe
# else
#  error no option for address size
# endif
#else
    typedef uint8_t FeldbusSize_t;
# if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 1
#  define TURAG_FELDBUS_NO_ANSWER 0xff
# elif TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
#  define TURAG_FELDBUS_NO_ANSWER 0xfe
# else
#  error no option for address size
# endif
#endif

#if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 1
    typedef uint8_t FeldbusAddress_t;
#elif TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
   typedef uint16_t FeldbusAddress_t;
#else
# error no option for address size
#endif


#endif // (!defined(__DOXYGEN__))


#endif // FELDBUS_CONFIG_CHECK_AVR_H_
