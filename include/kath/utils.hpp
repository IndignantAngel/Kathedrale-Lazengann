﻿#pragma once

#include "detail/demangle.hpp"

namespace kath
{
    struct stack_guard
    {
        stack_guard(lua_State* L)
            : L_(L)
            , top_(::lua_gettop(L))
            , dismiss_(false)
        {}

        ~stack_guard()
        {
            if(!dismiss_)
                ::lua_settop(L_, top_);
        }

        stack_guard(stack_guard const&) = delete;
        stack_guard(stack_guard&&) = delete;

        stack_guard& operator=(stack_guard const&) = delete;
        stack_guard& operator=(stack_guard&&) = delete;

        void dismiss() noexcept { dismiss_ = true; }
    
        lua_State*  L_;
        int         top_;
        bool        dismiss_;
    };

    template <typename ... Args>
    inline static auto tie(Args& ... args) noexcept -> tuple<Args&...>
    {
        return { args... };
    }

    // for tie
    template <typename ... Args>
    class tuple
    {
        using value_tuple = std::tuple<std::remove_reference_t<Args>...>;

    public:
        tuple(Args ... args)
            : t_(args...)
        {
        }

        template <typename InvokeExpr>
        void operator= (InvokeExpr const& expr)
        {
            t_ = static_cast<value_tuple>(expr);
        }

    private:
        std::tuple<Args...> t_; 
    };

    template <typename ... Args>
    struct type_list {};

    template <typename T, typename TypeList>
    struct type_list_push_front;
    template <typename T, typename ... Args>
    struct type_list_push_front<T, type_list<Args...>>
    {
        using type = type_list<T, Args...>;
    };
    template <typename T, typename TypeList>
    using type_list_push_front_t = typename type_list_push_front<T, TypeList>::type;
}

#ifndef KATH_MAKE_TYPE_LIST
#define KATH_MAKE_TYPE_LIST(...)  kath::type_list<__VA_ARGS__>{}
#endif

#ifndef KATH_RESULT
#define KATH_RESULT KATH_MAKE_TYPE_LIST
#endif

#ifndef KATH_ARGS
#define KATH_ARGS KATH_MAKE_TYPE_LIST
#endif

// some useful interface
namespace kath
{
    // TODO: remove class and struct
    template <typename T>
    inline static std::string const& get_class_name() noexcept
    {
        static std::string const name = detail::demangle<T>();
        return name;
    }

    // TODO ... flatten the gap between lua and C++
    template <typename T, bool Safe = false>
    inline static auto get_type_name() noexcept
    {
        using type = std::remove_const_t<std::remove_reference_t<T>>;

        if constexpr(is_bool_v<type>)
        {
            return 'b';
        }
        else if constexpr(meta_or_v<is_floating_point<type>, is_integral<type>>)
        {
            return 'n';
        }
        else if constexpr(is_string_v<type>)
        {
            return 's';
        }
        else
        {
            return 'x';
            //if constexpr(Safe)
            //{
            //    
            //}
            //else
            //{
            //    using raw_type = std::remove_pointer_t<type>;
            //    return get_class_name<raw_type>();
            //}
        }
    }
}