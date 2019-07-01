#ifndef UHSM_UTILS_H_
#define UHSM_UTILS_H_

#include <type_traits>
#include <tuple>
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
  
  // Compile-time tests
  ////////////////////////////////////////////////////////////////////////////////
  
  namespace Common
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
  
  namespace TestContains_DoContain_ReturnTrue
  {
    using Bool_contains = contains_t<int, Common::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::true_type>);
  }
  
  namespace TestContains_DoNotContain_ReturnFalse
  {
    using Bool_contains = contains_t<char, Common::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::false_type>);
  }
  
  namespace TestAddUnique_AddAlreadyPresent_TupleUnchanged
  {
    using New_tuple = add_unique_t<float, Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Common::Basic_tuple>);
  }
  
  namespace TestAddUnique_AddNotPresent_AddedAtHead
  {
    using New_tuple = add_unique_t<char, Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, bool, int, float>>);
  }
  
  namespace TestRemoveDuplicates_NoDuplicates_TupleUnchanged
  {
    using New_tuple = remove_duplicates_t<Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Common::Basic_tuple>);
  }
  
  namespace TestRemoveDuplicates_DuplicatesPresent_DuplicatesRemoved
  {
    using New_tuple = remove_duplicates_t<Common::Tuple_w_duplicates>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, int, bool, float>>);
  }
  
  namespace TestPrepend_PrependNewType_ReturnPrepended
  {
    struct A {};
    struct B {};
    struct C {};
    using New_tuple = prepend_t<A, std::tuple<B, C>>;
    static_assert(std::is_same_v<New_tuple, std::tuple<A, B, C>>);
  }
  
  namespace TestFlattenBy1St_NestedTuple_ReturnFlatTupleWith1StType
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
  
  namespace TestFlattenBy1St_TransitionTable_ReturnStateTuple
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
}

#endif
