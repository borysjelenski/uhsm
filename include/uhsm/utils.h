#ifndef UHSM_UTILS_H
#define UHSM_UTILS_H

#include <type_traits>
#include <tuple>
#include "uhsm/fwd.h"

namespace ushm::utils
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
  
//  template<typename MatchStateT, typename TransitionsT>
//  struct contains_src_state {
//    using type = std::false_type;
//  };
//  template<typename MatchStateT, typename... TrElemTs, typename... TailTrTs>
//  struct contains_src_state<MatchStateT, uhsm::Transition_table<uhsm::Transition<MatchStateT, TrElemTs...>, TailTrTs...>> {
//    using type = std::true_type;
//  };
//  template<typename MatchStateT, typename StateT, typename... TrElemTs, typename... TailTrTs>
//  struct contains_src_state<MatchStateT, uhsm::Transition_table<uhsm::Transition<StateT, TrElemTs...>, TailTrTs...>> {
//    using type = std::false_type;
//  };
//  template<typename MatchStateT, typename TransitionsT>
//  using contains_src_state_t = typename contains_src_state<MatchStateT, TransitionsT>::type;
  
  template<typename TransitionT, typename TransitionTableT>
  struct has_tr_w_src_state {
    using type = std::false_type;
  };
  template<typename MatchStateT, typename... Tr1ElemsTs, typename... Tr2ElemTs, typename... TailTrTs>
  struct has_tr_w_src_state<uhsm::Transition<MatchStateT, Tr1ElemsTs...>,
    uhsm::Transition_table<uhsm::Transition<MatchStateT, Tr2ElemTs...>, TailTrTs...>> {
    using type = std::true_type;
  };
  template<typename MatchStateT, typename StateT, typename... Tr1ElemTs, typename... Tr2ElemTs, typename... TailTrTs>
  struct has_tr_w_src_state<uhsm::Transition<MatchStateT, Tr1ElemTs...>,
    uhsm::Transition_table<uhsm::Transition<StateT, Tr2ElemTs...>, TailTrTs...>> {
    using type = std::false_type;
  };
  template<typename MatchStateT, typename TransitionTableT>
  using has_tr_w_src_state_t = typename has_tr_w_src_state<MatchStateT, TransitionTableT>::type;
  
//    template<typename PredT, typename TupleT, typename = typename Pred<>>
//    struct any_match {
//      using type = std::false_type;
//    };
//    
//    template<typename PredT, typename HeadT, typename... TailTs>
//    struct any_match<PredT, std::tuple<HeadT, TailTs...>> {
//      using 
//    };
  
//    // NOTE: uniquely adds a type at the original tuple's head
//  template<typename AddedT, typename TupleT, typename = typename contains<AddedT, TupleT>::type>
//  struct add_unique;
//  // original tuple does contain the type being added, leave original tuple unchanged
//  template<typename AddedT, typename... TupleTs>
//  struct add_unique<AddedT, std::tuple<TupleTs...>, std::true_type> {
//    using type = std::tuple<TupleTs...>;
//  };
//  // original tuple does not contain the type being added, add it at the tuple's head
//  template<typename AddedT, typename... TupleTs>
//  struct add_unique<AddedT, std::tuple<TupleTs...>, std::false_type> {
//    using type = std::tuple<AddedT, TupleTs...>;
//  };
  
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
  template<typename TransitionT, typename TransitionTableT>
  using add_unique_tr_w_src_state_t = typename add_unique<has_tr_w_src_state, TransitionT, TransitionTableT>::type;
  
//  // NOTE: removes duplicates from an std::tuple  
//  template<typename TupleT>
//  struct remove_duplicates;
//  // uniquely add current type to the tuple's tail with already removed duplicates
//  template<typename HeadT, typename... TailTs>
//  struct remove_duplicates<std::tuple<HeadT, TailTs...>>  {
//    using type = add_unique_t<HeadT, typename remove_duplicates<std::tuple<TailTs...>>::type>;
//  };
//  // a tuple with a single type cannot have duplicates
//  template<typename TailT>
//  struct remove_duplicates<std::tuple<TailT>> {
//    using type = std::tuple<TailT>;
//  };
//  
//  template<typename TupleT>
//  using remove_duplicates_t = typename remove_duplicates<TupleT>::type;
  
    // NOTE: removes duplicates from an std::tuple  
  template<typename TupleT>
  struct remove_duplicates;
  // uniquely add current type to the tuple's tail with already removed duplicates
  template<typename HeadT, typename... TailTs>
  struct remove_duplicates<std::tuple<HeadT, TailTs...>>  {
    using type = add_unique_t<HeadT, typename remove_duplicates<std::tuple<TailTs...>>::type>;
  };
  // a tuple with a single type cannot have duplicates
  template<typename TailT>
  struct remove_duplicates<std::tuple<TailT>> {
    using type = std::tuple<TailT>;
  };
  
  template<typename TupleT>
  using remove_duplicates_t = typename remove_duplicates<TupleT>::type;
  
  
  
  
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
  
  namespace TestHasTrWSrcState_DoesNotContain_ReturnFalse
  {
    struct Unknown_state {};
    using Tested_tr = uhsm::Transition<Unknown_state, Common::Pwr_btn_pressed, Common::On>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Common::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::false_type>);
  }
  
  namespace TestContainsSrcState_DoesContain_ReturnTrue
  {
    struct Unknown_state {};
    // NOTE: it does not matter that the transition table does not have a transition
    // exactly like the following one; only matching source types matters
    using Tested_tr = uhsm::Transition<Common::Off, Common::Pwr_btn_pressed, Unknown_state>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Common::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::true_type>);
  }
  
  namespace TestAddUniqueState_AddAlreadyPresent_TableUnchanged
  {
    using New_transition = uhsm::Transition<Common::Off, Common::Brownout, Common::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Common::Transitions>;
    // NOTE: new transition not added to the table because there is already a transition
    // with 'Off' source state
    static_assert(std::is_same_v<New_table, Common::Transitions>);
  }
}

#endif
