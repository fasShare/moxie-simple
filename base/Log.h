#ifndef MOXIE_LOG_H
#define MOXIE_LOG_H
#include <iostream>
#include <string.h>
#include <errno.h>

namespace moxie {

#define LOGGER_TRACE(MSG)
#define LOGGER_DEBUG(MSG)
#define LOGGER_INFO(MSG)
#define LOGGER_WARN(MSG)
#define LOGGER_ERROR(MSG)
#define LOGGER_FATAL(MSG)
#define LOGGER_SYSERR(MSG)

/*
#define LOGGER_TRACE(MSG) (std::cout << MSG << std::endl)
#define LOGGER_DEBUG(MSG) (std::cout << MSG << std::endl)
#define LOGGER_INFO(MSG) (std::cout << MSG << std::endl)
#define LOGGER_WARN(MSG) (std::cout << MSG << std::endl)
#define LOGGER_ERROR(MSG) (std::cout << MSG << std::endl)
#define LOGGER_FATAL(MSG) (std::cout << MSG << std::endl)
#define LOGGER_SYSERR(MSG) (std::cout << MSG << std::endl)
*/
}
#endif // MOXIE_LOG_H

