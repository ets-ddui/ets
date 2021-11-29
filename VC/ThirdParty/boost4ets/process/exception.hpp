// Copyright (c) 2016 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_EXCEPTION_HPP_
#define BOOST_PROCESS_EXCEPTION_HPP_

#include <system_error>

namespace ets
{
namespace process
{
///The exception usually thrown by boost.process.
/** It merely inherits [std::system_error](http://en.cppreference.com/w/cpp/error/system_error)
 * but can then be distinguished in the catch-block from other system errors.
 *
 */
struct process_error : std::system_error
{
    explicit process_error(std::error_code _Errcode, const std::string& _Message = "")
        : std::system_error(_Errcode, _Message)
    {
    }

    process_error(std::error_code::value_type _Errval, const std::error_category& _Errcat, const std::string& _Message = "")
        : std::system_error(_Errval, _Errcat, _Message)
    {
    }
};

}
}



#endif /* BOOST_PROCESS_EXCEPTION_HPP_ */
