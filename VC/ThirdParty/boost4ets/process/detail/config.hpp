// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
// Copyright (c) 2016 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/**
 * \file boost/process/config.hpp
 *
 * Defines various macros.
 */

#ifndef BOOST_PROCESS_DETAIL_CONFIG_HPP
#define BOOST_PROCESS_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <system_error>
#include <boost/system/api_config.hpp>

#include <process/exception.hpp>

#if defined(BOOST_POSIX_API)
#include <errno.h>
#if defined(__GLIBC__)
#include <features.h>
#else
extern char **environ;
#endif
#elif defined(BOOST_WINDOWS_API)
#include <boost/detail/winapi/get_last_error.hpp>
#else
#error "System API not supported by boost.process"
#endif

namespace ets { namespace process { namespace detail
{

#if !defined(BOOST_PROCESS_PIPE_SIZE)
#define BOOST_PROCESS_PIPE_SIZE 1024
#endif

#if defined(BOOST_POSIX_API)
namespace posix {namespace extensions {}}
namespace api = posix;

inline std::error_code get_last_error() noexcept
{
    return std::error_code(errno, std::system_category());
}

//copied from linux spec.
#if defined (__USE_XOPEN_EXTENDED) && !defined (__USE_XOPEN2K8) || defined( __USE_BSD)
#define BOOST_POSIX_HAS_VFORK 1
#endif

#elif defined(BOOST_WINDOWS_API)
namespace windows {namespace extensions {}}
namespace api = windows;

inline std::error_code get_last_error() BOOST_NOEXCEPT
{
    return std::error_code(::boost::detail::winapi::GetLastError(), std::system_category());
}
#endif

inline void throw_last_error(const std::string & msg)
{
    throw process_error(get_last_error(), msg);
}

inline void throw_last_error()
{
    throw process_error(get_last_error());
}


template<typename Char> BOOST_CONSTEXPR_OR_CONST Char null_char();
template<> BOOST_CONSTEXPR_OR_CONST char     null_char<char>     (){return   '\0';}
template<> BOOST_CONSTEXPR_OR_CONST wchar_t  null_char<wchar_t>  (){return  L'\0';}

template<typename Char> BOOST_CONSTEXPR_OR_CONST Char equal_sign();
template<> BOOST_CONSTEXPR_OR_CONST char     equal_sign<char>    () {return  '='; }
template<> BOOST_CONSTEXPR_OR_CONST wchar_t  equal_sign<wchar_t> () {return L'='; }

template<typename Char> BOOST_CONSTEXPR_OR_CONST Char quote_sign();
template<> BOOST_CONSTEXPR_OR_CONST char     quote_sign<char>    () {return  '"'; }
template<> BOOST_CONSTEXPR_OR_CONST wchar_t  quote_sign<wchar_t> () {return L'"'; }

template<typename Char> BOOST_CONSTEXPR_OR_CONST Char space_sign();
template<> BOOST_CONSTEXPR_OR_CONST char     space_sign<char>    () {return  ' '; }
template<> BOOST_CONSTEXPR_OR_CONST wchar_t  space_sign<wchar_t> () {return L' '; }


}}}
#endif
