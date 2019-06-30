#ifndef UHSM_HELPERS_H_
#define UHSM_HELPERS_H_

#include "uhsm/utils.h"

namespace ushm::helpers
{
  // checks if uhsm::Transition_table contains a transition with a given source state;
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
  
  // TODO: rename to `add_unique_tr_by_src`
  template<typename TransitionT, typename TransitionTableT>
  using add_unique_tr_w_src_state_t = typename utils::add_unique<has_tr_w_src_state, TransitionT, TransitionTableT>::type;
  
  template<typename TransitionTableT>
  using rm_dupl_tr_by_src_t = typename utils::remove_duplicates<has_tr_w_src_state, TransitionTableT>::type;
  
  // TODO: currently duplicated in both utils.h and helpers.h
  namespace Test_data
  {
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
  
  namespace TestHasTrWSrcState_DoesNotContain_ReturnFalse
  {
    struct Unknown_state {};
    using Tested_tr = uhsm::Transition<Unknown_state, Test_data::Pwr_btn_pressed, Test_data::On>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Test_data::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::false_type>);
  }
  
  namespace TestHasTrWSrcState_DoesContain_ReturnTrue
  {
    struct Unknown_state {};
    // NOTE: it does not matter that the transition table does not have a transition
    // exactly like the following one; only matching source types matters
    using Tested_tr = uhsm::Transition<Test_data::Off, Test_data::Pwr_btn_pressed, Unknown_state>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Test_data::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::true_type>);
  }
  
  namespace TestAddUniqueTrWSrcState_AddAlreadyPresent_TableUnchanged
  {
    using New_transition = uhsm::Transition<Test_data::Off, Test_data::Brownout, Test_data::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Test_data::Transitions>;
    // NOTE: new transition not added to the table because there is already a transition
    // with 'Off' source state
    static_assert(std::is_same_v<New_table, Test_data::Transitions>);
  }
  
  namespace TestAddUniqueTrWSrcState_AddNotPresent_TransitionAdded
  {
    struct New_state {};
    using New_transition = uhsm::Transition<New_state, Test_data::Brownout, Test_data::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Test_data::Transitions>;
    using Expected_table = uhsm::Transition_table<
      New_transition,
      uhsm::Transition<Test_data::Off, Test_data::Pwr_btn_pressed, Test_data::On>,
      uhsm::Transition<Test_data::On, Test_data::Pwr_btn_pressed, Test_data::Off>,
      uhsm::Transition<Test_data::On, Test_data::Brownout, Test_data::Off>
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
