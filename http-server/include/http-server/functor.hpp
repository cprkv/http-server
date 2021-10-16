#pragma once
#include "http-server/pch.hpp"

namespace http {
  template <typename TReturn, typename... TArguments>
  struct FunctorInfoBase {
    using ReturnType = TReturn;

    template <size_t i>
    struct ArgumentAt {
      using Type = typename std::tuple_element<i, std::tuple<TArguments...>>::type;
    };
  };

  template <typename TFunctor>
  struct FunctorInfo : FunctorInfo<decltype(&TFunctor::operator())> {};

  template <typename TReturn, typename TFunctorClass, typename... TArguments>
  struct FunctorInfo<TReturn (TFunctorClass::*)(TArguments...)>
      : FunctorInfoBase<TReturn, TArguments...> {};

  template <typename TReturn, typename TFunctorClass, typename... TArguments>
  struct FunctorInfo<TReturn (TFunctorClass::*)(TArguments...) const>
      : FunctorInfoBase<TReturn, TArguments...> {};

  template <typename T>
  struct UnitFunctor {
    constexpr T operator()(T value) { return value; }
  };
} // namespace http
