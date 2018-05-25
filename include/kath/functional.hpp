#pragma once

// placeholders
namespace kath
{
	namespace upvalue_placeholders
	{
		template <int I>
		struct placeholder : std::integral_constant<int ,I>
		{
			operator int() const noexcept
			{
				return lua_upvalueindex(I);
			}
		};

		inline constexpr placeholder<1> _1{};
		inline constexpr placeholder<2> _2{};
		inline constexpr placeholder<3> _3{};
		inline constexpr placeholder<4> _4{};
		inline constexpr placeholder<5> _5{};
		inline constexpr placeholder<6> _6{};
		inline constexpr placeholder<7> _7{};
		inline constexpr placeholder<8> _8{};
		inline constexpr placeholder<9> _9{};
	}

	namespace register_placeholders
	{
		constexpr struct placehodler
		{
			operator int() const noexcept
			{
				return LUA_REGISTRYINDEX;
			}
		}
		_reg {};
	}
}

// bind
namespace kath
{
	namespace detail
	{
		struct bind_tag_unknown{};
		struct bind_tag_pmf{};
		struct bind_tag_pmd{};
		struct bind_tag_callable{};

		template <typename T>
		inline static constexpr auto get_bind_tag() noexcept
		{
			using type = std::remove_reference_t<T>;

			if constexpr(std::is_member_function_pointer_v<type>)
			{
				return bind_tag_pmf{};
			}
			else if constexpr(std::is_member_object_pointer_v<type>)
			{
				return bind_tag_pmd{};
			}
			else if constexpr(is_callable_v<type>)
			{
				return bind_tag_callable{};
			}
			else 
			{
				return bind_tag_unknown{};
			}
		}

		template <typename Tag>
		struct bind_helper;

		template <>
		struct bind_helper<bind_tag_unknown>;

		template <>
		struct bind_helper<bind_tag_pmf>
		{
            //template <typename Pmd, typename ... Args>
            //auto operator()(Pmd pmf, )
		};

		template <>
		struct bind_helper<bind_tag_pmd>
		{
            
		};

		template <>
		struct bind_helper<bind_tag_callable>
		{
            template <typename Func, typename ... Args>
            decltype(auto) operator() (Func&& func, Args&& ... args)
            {
                using callable_traits_t = callable_traits<Func>;

                if constexpr(sizeof...(Args) == 0)
                {
                    return func;
                }
                else
                {
                    static_assert(sizeof...(Args) == callable_traits_t::arity, "Parametres number not match!");
                }
            }

        private:
            template <typename Func, typename Tuple>
            auto bind_impl(Func&& func, Tuple&& tuple)
            {

            }
		};
	}

	template <typename Func, typename ... Args>
	inline static auto bind(Func&& f, Args&& ... args)
	{
		
	}
}


// lua_cfunctor
namespace kath
{
	template <typename Func>
	struct lua_cfunctor
	{
		using function_type = std::remove_reference_t<Func>;
		using callable_traits_t = callable_traits<Func>;
		using result_type = typename callable_traits_t::result_type;
		using args_pack = typename callable_traits_t::args_pack;
		static constexpr size_t arity = callable_traits_t::arity;

	public:
		static void stack_push_callable(lua_State* L, Func func)
		{
			stack_push_impl(L, std::forward<function_type>(func));
		}

	private:
		template <typename T>
		static auto invoke_impl(lua_State* L, function_type const& func)
			-> std::enable_if_t<std::is_void_v<T>, int> 
		{
			invoke_impl(L, func, std::make_index_sequence<arity>{});
			return 0;
		}

		template <typename T>
		static auto invoke_impl(lua_State* L, function_type const& func)
			-> disable_if_t<std::is_void_v<T>, int>
		{
			auto result = invoke_impl(L, func, std::make_index_sequence<arity>{});
			stack_push(L, std::move(result));
			return 1;
		}

		template <size_t ... Is>
		static auto invoke_impl(lua_State* L, function_type const& func, std::index_sequence<Is...>)
		{
			return func(stack_check<std::tuple_element_t<Is, args_pack>>(L, Is + 1)...);
		}

		static int invoke(lua_State* L)
		{
			using upvalue_placeholders::_1;
			auto callable = stack_get<function_type>(L, _1);
			return invoke_impl<result_type>(L, *callable);
		}

		static void stack_push_impl(lua_State* L, lua_CFunction f)
		{
			::lua_pushcclosure(L, f, 0);
		}

		template <typename F>
		static void stack_push_impl(lua_State* L, F&& f)
		{
			detail::stack_push_userdata(L, std::forward<F>(f));

			// TODO ... metatable for destructor

			::lua_pushcclosure(L, &lua_cfunctor::invoke, 1);
		}
	};
}

// lua callable