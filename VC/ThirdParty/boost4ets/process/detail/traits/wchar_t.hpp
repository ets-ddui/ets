// Copyright (c) 2016 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_PROCESS_DETAIL_TRAITS_WCHAR_T_HPP_
#define BOOST_PROCESS_DETAIL_TRAITS_WCHAR_T_HPP_

#include <process/detail/traits/decl.hpp>
#include <process/detail/traits/cmd_or_exe.hpp>
#include <process/detail/traits/env.hpp>
#include <process/locale.hpp>

namespace ets { namespace process { namespace detail {

//template

template<typename T> struct is_wchar_t : std::false_type {};

template<> struct is_wchar_t<boost::filesystem::path> : std::is_same<typename boost::filesystem::path::value_type, wchar_t>
{
};

template<> struct is_wchar_t<const wchar_t* > : std::true_type {};

template<> struct is_wchar_t<wchar_t* > : std::true_type {};

template<std::size_t Size> struct is_wchar_t<const wchar_t [Size]>    : std::true_type {};
template<std::size_t Size> struct is_wchar_t<const wchar_t (&)[Size]> : std::true_type {};
template<std::size_t Size> struct is_wchar_t<wchar_t [Size]>    : std::true_type {};

template<> struct is_wchar_t<std::wstring>               : std::true_type {};
template<> struct is_wchar_t<std::vector<std::wstring>>  : std::true_type {};
template<> struct is_wchar_t<std::initializer_list<std::wstring>> : std::true_type {};
template<> struct is_wchar_t<std::vector<wchar_t *>>           : std::true_type {};
template<> struct is_wchar_t<std::initializer_list<wchar_t *>> : std::true_type {};



template<typename Char, typename T>
struct char_converter
{
    template<typename Element>
    struct result
    {
        typedef Element & type;
    };

    static T&  conv(T & in)
    {
        return in;
    }
    /*
    static T&& conv(T&& in)
    {
        return std::move(in);
    }
    static const T&  conv(const T & in)
    {
        return in;
    }
    */
};
/*
template<typename Char, typename T>
using char_converter_t = char_converter<Char,
        typename std::remove_cv<typename std::remove_reference<T>::type>::type>;
*/

template<>
struct char_converter<char, const wchar_t*>
{
    typedef std::string result_type;

    static std::string conv(const wchar_t* in)
    {
        std::size_t size = 0;
        while (in[size] != L'\0') size++;
        return ::ets::process::detail::convert(in, in + size);
    }
};

template<>
struct char_converter<char, wchar_t*>
{
    typedef std::string result_type;

    static std::string conv(wchar_t* in)
    {
        std::size_t size = 0;
        while (in[size] != L'\0') size++;
        return ::ets::process::detail::convert(in, in + size);
    }
};

template<std::size_t Size>
struct char_converter<char, wchar_t[Size]>
{
    typedef std::string result_type;

    static std::string conv(const wchar_t(&in)[Size])
    {
        return ::ets::process::detail::convert(in, in + Size -1);
    }
};

template<>
struct char_converter<wchar_t, const char*>
{
    typedef std::wstring result_type;

    static std::wstring conv(const char* in)
    {
        std::size_t size = 0;
        while (in[size] != '\0') size++;
        return ::ets::process::detail::convert(in, in + size);
    }
};

template<>
struct char_converter<wchar_t, char*>
{
    typedef std::wstring result_type;

    static std::wstring conv(char* in)
    {
        std::size_t size = 0;
        while (in[size] != '\0') size++;
        return ::ets::process::detail::convert(in, in + size);
    }
};


template<std::size_t Size>
struct char_converter<wchar_t, char[Size]>
{
    typedef std::wstring result_type;

    static std::wstring conv(const char(&in)[Size])
    {
        return ::ets::process::detail::convert(in, in + Size -1);
    }
};

template<std::size_t Size>
struct char_converter<wchar_t, const char[Size]>
{
    typedef std::wstring result_type;

    static std::wstring conv(const char(&in)[Size])
    {
        return ::ets::process::detail::convert(in, in + Size -1);
    }
};

//all the containers.
template<>
struct char_converter<wchar_t, std::string>
{
    typedef std::wstring result_type;

    static std::wstring conv(const std::string & in)
    {
        return ::ets::process::detail::convert(in);
    }
};

template<>
struct char_converter<wchar_t, const std::string>
{
    typedef std::wstring result_type;

    static std::wstring conv(const std::string & in)
    {
        return ::ets::process::detail::convert(in);
    }
};

template<>
struct char_converter<char, std::wstring>
{
    typedef std::string result_type;

    static std::string conv(const std::wstring & in)
    {
        return ::ets::process::detail::convert(in);
    }
};

template<>
struct char_converter<wchar_t, std::vector<std::string>>
{
    typedef std::vector<std::wstring> result_type;

    static std::vector<std::wstring> conv(const std::vector<std::string> & in)
    {
        std::vector<std::wstring> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const std::string & st) -> std::wstring
                {
                    return convert(st);
                });
        return ret;
    }
};

template<>
struct char_converter<wchar_t, std::initializer_list<std::string>>
{
    typedef std::vector<std::wstring> result_type;

    static std::vector<std::wstring> conv(const std::initializer_list<std::string> & in)
    {
        std::vector<std::wstring> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const std::string & st) -> std::wstring
                {
                    return convert(st);
                });
        return ret;
    }
};

template<>
struct char_converter<wchar_t, std::vector<char* >>
{
    typedef std::vector<std::wstring> result_type;

    static std::vector<std::wstring> conv(const std::vector<char* > & in)
    {
        std::vector<std::wstring> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const char* st) -> std::wstring
                {
                    std::size_t sz = 0;
                    while (st[sz] != '\0') sz++;
                    return convert(st, st + sz);
                });
        return ret;
    }
};

template<>
struct char_converter<wchar_t, std::initializer_list<char *>>
{
    typedef std::vector<std::wstring> result_type;

    static std::vector<std::wstring>  conv(const std::initializer_list<char * > & in)
    {
        std::vector<std::wstring> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const char* st) -> std::wstring
                {
                    std::size_t sz = 0;
                    while (st[sz] != '\0') sz++;
                    return convert(st, st + sz);
                });
        return ret;
    }
};

template<>
struct char_converter<char, std::vector<std::wstring>>
{
    typedef std::vector<std::string> result_type;

    static std::vector<std::string> conv(const std::vector<std::wstring> & in)
    {
        std::vector<std::string> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const std::wstring & st) -> std::string
                {
                    return convert(st);
                });
        return ret;
    }
};

template<>
struct char_converter<char, std::initializer_list<std::wstring>>
{
    typedef std::vector<std::string> result_type;

    static std::vector<std::string> conv(const std::initializer_list<std::wstring> & in)
    {
        std::vector<std::string> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const std::wstring & st) -> std::string
                {
                    return convert(st);
                });
        return ret;
    }
};

template<>
struct char_converter<char, std::vector<wchar_t* >>
{
    typedef std::vector<std::string> result_type;

    static std::vector<std::string> conv(const std::vector<wchar_t* > & in)
    {
        std::vector<std::string> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const wchar_t* st) -> std::string
                {
                    std::size_t sz = 0;
                    while (st[sz] != L'\0') sz++;
                    return convert(st, st + sz);
                });
        return ret;
    }
};

template<>
struct char_converter<char, std::initializer_list<wchar_t * >>
{
    typedef std::vector<std::string> result_type;

    static std::vector<std::string> conv(const std::initializer_list<wchar_t *> & in)
    {
        std::vector<std::string> ret(in.size());
        std::transform(in.begin(), in.end(), ret.begin(),
                [](const wchar_t* st) -> std::string
                {
                    std::size_t sz = 0;
                    while (st[sz] != L'\0') sz++;
                    return convert(st, st + sz);
                });
        return ret;
    }
};


}}}
#endif /* BOOST_PROCESS_DETAIL_TRAITS_WCHAR_T_HPP_ */
