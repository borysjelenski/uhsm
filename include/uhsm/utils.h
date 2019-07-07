#ifndef UHSM_UTILS_H_
#define UHSM_UTILS_H_

#include <cassert>
#include <type_traits>
#include <tuple>
#include <variant>
#include <utility>
#include "uhsm/fwd.h"

namespace uhsm::utils
{ 
  // NOTE: checks if a std::tuple contains a type; assumes that it does not
  // unless 'proven' otherwise
  template<typename MatchT, typename TupleT>
  struct contains {
    using type = std::false_type;
  };
  // matched type at tuple's head, true
  template<typename HeadT, typename... TailTs>
  struct contains<HeadT, std::tuple<HeadT, TailTs...>> {
    using type = std::true_type;
  };
  // not matched at tuple's head, check the tail
  template<typename MatchT, typename HeadT, typename... TailTs>
  struct contains<MatchT, std::tuple<HeadT, TailTs...>> {
    using type = typename contains<MatchT, std::tuple<TailTs...>>::type;
  };
  
  template<typename MatchT, typename TupleT>
  using contains_t = typename contains<MatchT, TupleT>::type;
  
  // NOTE: uniquely adds a type at the original tuple's head
  template<template<class, class> class EqualPred, typename AddedT, typename TupleT,
    typename = typename EqualPred<AddedT, TupleT>::type>
  struct add_unique;
  // original tuple does contain the type being added, leave original tuple unchanged
  template<template<class, class> class EqualPred, typename AddedT, typename... TupleTs>
  struct add_unique<EqualPred, AddedT, std::tuple<TupleTs...>, std::true_type> {
    using type = std::tuple<TupleTs...>;
  };
  // original tuple does not contain the type being added, add it at the tuple's head
  template<template<class, class> class EqualPred, typename AddedT, typename... TupleTs>
  struct add_unique<EqualPred, AddedT, std::tuple<TupleTs...>, std::false_type> {
    using type = std::tuple<AddedT, TupleTs...>;
  };
  
  template<typename AddedT, typename TupleT>
  using add_unique_t = typename add_unique<contains, AddedT, TupleT>::type;
  
  // TODO: change name to `rm_dupl`
  // NOTE: removes duplicates from an std::tuple
  template<template<class, class> class EqPred, typename TupleT>
  struct remove_duplicates;
  // uniquely add current type to the tuple's tail with already removed duplicates
  template<template<class, class> class EqPred, typename HeadT, typename... TailTs>
  struct remove_duplicates<EqPred, std::tuple<HeadT, TailTs...>>  {
    using type = typename add_unique<EqPred, HeadT, typename remove_duplicates<EqPred, std::tuple<TailTs...>>::type>::type;
  };
  // a tuple with a single type cannot have duplicates
  template<template<class, class> class EqPred, typename TailT>
  struct remove_duplicates<EqPred, std::tuple<TailT>> {
    using type = std::tuple<TailT>;
  };
  
  template<typename TupleT>
  using remove_duplicates_t = typename remove_duplicates<contains, TupleT>::type;
  
  template<typename AddedT, typename TupleT>
  struct prepend;
  template<typename AddedT, typename... TupleTs>
  struct prepend<AddedT, std::tuple<TupleTs...>> {
    using type = std::tuple<AddedT, TupleTs...>;
  };
  
  template<typename AddedT, typename TupleT>
  using prepend_t = typename prepend<AddedT, TupleT>::type;
  
  // gets index of a type in a tuple's type list
  template<size_t N, typename MatchT, typename TupleT>
  struct tuple_elem_idx_impl;
  template<size_t N, typename MatchT, typename... TailTs>
  struct tuple_elem_idx_impl<N, MatchT, std::tuple<MatchT, TailTs...>>  {
    static constexpr size_t value = N;
  };
  template<size_t N, typename MatchT, typename HeadT, typename... TailTs>
  struct tuple_elem_idx_impl<N, MatchT, std::tuple<HeadT, TailTs...>> {
    static constexpr size_t value = tuple_elem_idx_impl<N + 1, MatchT, std::tuple<TailTs...>>::value;
  };
  
  template<typename MatchT, typename TupleT>
  inline constexpr size_t tuple_elem_idx_v = tuple_elem_idx_impl<0, MatchT, TupleT>::value;
  
  template<typename TupleT>
  struct flatten_by_1st;  
  template<typename MatchT, typename... OtherTs, typename... TailTupleTs>
  struct flatten_by_1st<std::tuple<std::tuple<MatchT, OtherTs...>, TailTupleTs...>> {
    using type = typename prepend<MatchT, typename flatten_by_1st<std::tuple<TailTupleTs...>>::type>::type;
  }; 
  template<typename MatchT, typename... OtherTs>
  struct flatten_by_1st<std::tuple<std::tuple<MatchT, OtherTs...>>> { 
    using type = std::tuple<MatchT>;
  };
  
  template<typename TupleT>
  using flatten_by_1st_t = typename flatten_by_1st<TupleT>::type;
  
  template<template<class...> class Func, typename TupleT>
  struct apply_func;
  template<template<class...> class Func, typename... Ts>
  struct apply_func<Func, std::tuple<Ts...>> {
    using type = Func<Ts...>;
  };
  
  template<template<class...> class Func, typename TupleT>
  using apply_func_t = typename apply_func<Func, TupleT>::type;
  
  template<typename TupleT>
  using variant_from_tuple_ts = apply_func_t<std::variant, TupleT>;
  
  template<typename TupleT, typename HeadT, typename... TailTs>
  constexpr auto make_variant_by_index_impl(size_t idx)
  {
    using Variant_type = variant_from_tuple_ts<TupleT>;
    
    if (tuple_elem_idx_v<HeadT, TupleT> == idx) {
      return Variant_type{HeadT{}};
    }
    
    if constexpr(sizeof...(TailTs) > 0) {
      return make_variant_by_index_impl<TupleT, TailTs...>(idx);
    } else {
      // WARNING: passing an out-of-bounds index will result in returning a variant
      // with first alt. type
      // TODO: implement an error-handling mechanism
      return Variant_type{};
    }  
  }
  
  template<typename TupleT>
  struct Variant_by_index;
  template<typename HeadT, typename... TailTs>
  struct Variant_by_index<std::tuple<HeadT, TailTs...>> {
    static constexpr auto make(size_t idx)
    {
      return make_variant_by_index_impl<std::tuple<HeadT, TailTs...>, HeadT, TailTs...>(idx);
    }
  };

  template<typename VariantT, typename Func, typename ArgT, typename HeadT, typename... TailTs>
  constexpr void variant_invoke_impl(VariantT& var, ArgT&& arg)
  {
    if (std::holds_alternative<HeadT>(var)) {
      Func::invoke(std::get<HeadT>(var), std::forward<ArgT>(arg));
    }
     
    if constexpr (sizeof...(TailTs) > 0) {
      variant_invoke_impl<VariantT, Func, ArgT, TailTs...>(var, std::forward<ArgT>(arg));
    }
  }
  
  // invokes a functor on an alternative object currently held by the variant
  template<typename Func, typename VariantT>
  struct variant_invocation;
  template<typename Func, typename HeadT, typename... TailTs>
  struct variant_invocation<Func, std::variant<HeadT, TailTs...>> {
    template<typename ArgT>
    static constexpr void invoke(std::variant<HeadT, TailTs...>& var, ArgT&& arg)
    {
      variant_invoke_impl<std::variant<HeadT, TailTs...>, Func, ArgT, HeadT, TailTs...>(
        var, std::forward<ArgT>(arg));
    }
  };
  
  // Compile-time tests
  ////////////////////////////////////////////////////////////////////////////////
  
  namespace Test_data
  {
    using Basic_tuple = std::tuple<bool, int, float>;
    // NOTE: duplicate types occur both adjacently and non-adjacently on the parameter list
    using Tuple_w_duplicates = std::tuple<int, char, float, int, int, bool, float, float>;
    
    // States
    struct Off {};
    struct On {};
    // Events
    struct Pwr_btn_pressed {};
    struct Brownout {};
    
    using Transitions = uhsm::Transition_table<
      uhsm::Transition<Off, Pwr_btn_pressed, On>,
      uhsm::Transition<On, Pwr_btn_pressed, Off>,
      uhsm::Transition<On, Brownout, Off>
    >;
  }
  
  namespace Test::Contains_DoContain_ReturnTrue
  {
    using Bool_contains = contains_t<int, Test_data::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::true_type>);
  }
  
  namespace Test::Contains_DoNotContain_ReturnFalse
  {
    using Bool_contains = contains_t<char, Test_data::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::false_type>);
  }
  
  namespace Test::AddUnique_AddAlreadyPresent_TupleUnchanged
  {
    using New_tuple = add_unique_t<float, Test_data::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Test_data::Basic_tuple>);
  }
  
  namespace Test::AddUnique_AddNotPresent_AddedAtHead
  {
    using New_tuple = add_unique_t<char, Test_data::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, bool, int, float>>);
  }
  
  namespace Test::RemoveDuplicates_NoDuplicates_TupleUnchanged
  {
    using New_tuple = remove_duplicates_t<Test_data::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Test_data::Basic_tuple>);
  }
  
  namespace Test::RemoveDuplicates_DuplicatesPresent_DuplicatesRemoved
  {
    using New_tuple = remove_duplicates_t<Test_data::Tuple_w_duplicates>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, int, bool, float>>);
  }
  
  namespace Test::Prepend_PrependNewType_ReturnPrepended
  {
    struct A {};
    struct B {};
    struct C {};
    using New_tuple = prepend_t<A, std::tuple<B, C>>;
    static_assert(std::is_same_v<New_tuple, std::tuple<A, B, C>>);
  }
  
  namespace Test::TupleElemIdx_SimpleTuple_ReturnTypeIndex
  {
    struct A {};
    struct B {};
    struct C {};
    using Test_tuple = std::tuple<A, B, C>;
    static_assert(tuple_elem_idx_v<A, Test_tuple> == 0);
    static_assert(tuple_elem_idx_v<B, Test_tuple> == 1);
    static_assert(tuple_elem_idx_v<C, Test_tuple> == 2);
  }
  
  namespace Test::FlattenBy1St_NestedTuple_ReturnFlatTupleWith1StType
  {
    struct A {};
    struct B {};
    struct C {};
    using Nested_tuple = std::tuple<
      std::tuple<A, B, C>,
      std::tuple<B, A, C>,
      std::tuple<C, A, B>
    >;
    using Flat_tuple = flatten_by_1st_t<Nested_tuple>;
    static_assert(std::is_same_v<Flat_tuple, std::tuple<A, B, C>>);
  }
  
  namespace Test::FlattenBy1St_TransitionTable_ReturnStateTuple
  {
    struct StateA {};
    struct StateB {};
    struct StateC {};
    struct Event1 {};
    struct Event2 {};
    using Table = uhsm::Transition_table<
      uhsm::Transition<StateA, Event2, StateC>,
      uhsm::Transition<StateB, Event1, StateA>,
      uhsm::Transition<StateC, Event2, StateA>
    >;
    using State_tuple = flatten_by_1st_t<Table>;
    static_assert(std::is_same_v<State_tuple, std::tuple<StateA, StateB, StateC>>);
  }
  
  namespace Test::MakeVariantByIndex_InBoundIndex_MakeVariantWithIthAlt
  {
    struct A {};
    struct B {};
    struct C {};
    constexpr auto variant_a = Variant_by_index<std::tuple<A, B, C>>::make(0);
    constexpr auto variant_b = Variant_by_index<std::tuple<A, B, C>>::make(1);
    constexpr auto variant_c = Variant_by_index<std::tuple<A, B, C>>::make(2);
       
    using Expected_variant = const std::variant<A, B, C>;
    static_assert(std::is_same_v<decltype(variant_a), Expected_variant>);
    static_assert(std::is_same_v<decltype(variant_b), Expected_variant>);
    static_assert(std::is_same_v<decltype(variant_c), Expected_variant>);
    static_assert(variant_a.index() == 0);
    static_assert(variant_b.index() == 1);
    static_assert(variant_c.index() == 2);
  }
}

#endif
