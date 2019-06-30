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
  
  // NOTE: checks if uhsm::Transition_table contains a transition with a given source state;
  // works similarly to `contains` template
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
    using type = typename has_tr_w_src_state<uhsm::Transition<MatchStateT, Tr1ElemTs...>,
      uhsm::Transition_table<TailTrTs...>>::type;
  };
  
  template<typename MatchStateT, typename TransitionTableT>
  using has_tr_w_src_state_t = typename has_tr_w_src_state<MatchStateT, TransitionTableT>::type;
  
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

  // TODO: rename to `add_unique_tr_by_src`
  template<typename TransitionT, typename TransitionTableT>
  using add_unique_tr_w_src_state_t = typename add_unique<has_tr_w_src_state, TransitionT, TransitionTableT>::type;
  
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
  
  template<typename TransitionTableT>
  using rm_dupl_tr_by_src_t = typename remove_duplicates<has_tr_w_src_state, TransitionTableT>::type;
  
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
    
//    struct StateA {};
//    struct StateB {};
//    struct StateC {};
//    struct Event1 {};
//    struct Event2 {};
//    using Table = uhsm::Transition_table<
//      uhsm::Transition<StateA, Event2, StateC>,
//      uhsm::Transition<StateB, Event1, StateA>,
//      uhsm::Transition<StateC, Event2, StateA>
//    >;
//    using State_set = typename flatten_by_1st<Table>::type;
//    static_assert<std::is_same_v<State_set, std::tuple<StateA, StateB, StateC>>;
  }
  
  namespace TestHasTrWSrcState_DoesNotContain_ReturnFalse
  {
    struct Unknown_state {};
    using Tested_tr = uhsm::Transition<Unknown_state, Common::Pwr_btn_pressed, Common::On>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Common::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::false_type>);
  }
  
  namespace TestHasTrWSrcState_DoesContain_ReturnTrue
  {
    struct Unknown_state {};
    // NOTE: it does not matter that the transition table does not have a transition
    // exactly like the following one; only matching source types matters
    using Tested_tr = uhsm::Transition<Common::Off, Common::Pwr_btn_pressed, Unknown_state>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Common::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::true_type>);
  }
  
  namespace TestAddUniqueTrWSrcState_AddAlreadyPresent_TableUnchanged
  {
    using New_transition = uhsm::Transition<Common::Off, Common::Brownout, Common::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Common::Transitions>;
    // NOTE: new transition not added to the table because there is already a transition
    // with 'Off' source state
    static_assert(std::is_same_v<New_table, Common::Transitions>);
  }
  
  namespace TestAddUniqueTrWSrcState_AddNotPresent_TransitionAdded
  {
    struct New_state {};
    using New_transition = uhsm::Transition<New_state, Common::Brownout, Common::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Common::Transitions>;
    using Expected_table = uhsm::Transition_table<
      New_transition,
      uhsm::Transition<Common::Off, Common::Pwr_btn_pressed, Common::On>,
      uhsm::Transition<Common::On, Common::Pwr_btn_pressed, Common::Off>,
      uhsm::Transition<Common::On, Common::Brownout, Common::Off>
    >;
    static_assert(std::is_same_v<New_table, Expected_table>);
  }
  
  namespace TestRmDuplTrBySrc_NoDupl_TableUnchanged
  {
    struct StateA {};
    struct StateB {};
    struct Some_event {};
    using No_dupl_table = uhsm::Transition_table<
      uhsm::Transition<StateA, Some_event, StateB>,
      uhsm::Transition<StateB, Some_event, StateA>
    >;
    using New_table = rm_dupl_tr_by_src_t<No_dupl_table>;
    static_assert(std::is_same_v<New_table, No_dupl_table>);
  }
  
  namespace TestRmDuplTrBySrc_DuplPresent_DuplRemoved
  {    
    struct StateA {};
    struct StateB {};
    struct StateC {};
    struct Event1 {};
    struct Event2 {};
    // NOTE: duplicate types occur both adjacently and on the parameter list
    using Dupl_table = uhsm::Transition_table<
      uhsm::Transition<StateC, Event2, StateA>,
      uhsm::Transition<StateA, Event1, StateB>,
      uhsm::Transition<StateA, Event2, StateC>,
      uhsm::Transition<StateB, Event2, StateA>,
      uhsm::Transition<StateB, Event1, StateA>,
      uhsm::Transition<StateC, Event1, StateA>,
      uhsm::Transition<StateB, Event1, StateA>,
      uhsm::Transition<StateC, Event2, StateA>
    >;
    using New_table = rm_dupl_tr_by_src_t<Dupl_table>;
    // NOTE: every last transition with an unique source state remains in the output table
    using Expected_table = uhsm::Transition_table<
      uhsm::Transition<StateA, Event2, StateC>,
      uhsm::Transition<StateB, Event1, StateA>,
      uhsm::Transition<StateC, Event2, StateA>
    >;
    static_assert(std::is_same_v<New_table, Expected_table>);
  }
}

#endif
