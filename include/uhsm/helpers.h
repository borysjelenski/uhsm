#ifndef UHSM_HELPERS_H_
#define UHSM_HELPERS_H_

#include <limits>
#include "uhsm/utils.h"

namespace uhsm::helpers
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
  
  template<typename TransitionT, typename TransitionTableT>
  using add_unique_tr_w_src_state_t = typename utils::add_unique<has_tr_w_src_state, TransitionT, TransitionTableT>::type;
  
  template<typename TransitionTableT>
  using rm_dupl_tr_by_src_t = typename utils::remove_duplicates<has_tr_w_src_state, TransitionTableT>::type;
  
  template<typename TransitionTableT>
  struct extract_state_set {
    using type = utils::flatten_by_1st_t<rm_dupl_tr_by_src_t<TransitionTableT>>;
  };
  
  template<typename TransitionTableT>
  using extract_state_set_t = typename extract_state_set<TransitionTableT>::type;
  
  // gets index of a state in a set of state (tuple)
  template<size_t N, typename MatchStateT, typename StateSetT>
  struct get_state_idx_impl;
  template<size_t N, typename MatchStateT, typename... TailStateTs>
  struct get_state_idx_impl<N, MatchStateT, std::tuple<MatchStateT, TailStateTs...>>  {
    static constexpr size_t value = N;
  };
  template<size_t N, typename MatchStateT, typename HeadStateT, typename... TailStateTs>
  struct get_state_idx_impl<N, MatchStateT, std::tuple<HeadStateT, TailStateTs...>> {
    static constexpr size_t value = get_state_idx_impl<N + 1, MatchStateT, std::tuple<TailStateTs...>>::value;
  };
  
  template<typename MatchStateT, typename StateSetT>
  inline constexpr size_t get_state_idx_v = get_state_idx_impl<0, MatchStateT, StateSetT>::value;
  
  template<typename TransitionT>
  using get_tr_src_state = std::tuple_element_t<0, TransitionT>;
  template<typename TransitionT>
  using get_tr_event = std::tuple_element_t<1, TransitionT>;
  template<typename TransitionT>
  using get_tr_dest_state = std::tuple_element_t<2, TransitionT>;
  template<typename TransitionT>
  using get_tr_action = std::tuple_element_t<3, TransitionT>;
  
  template<typename StateSetT, typename TransitionT>
  inline constexpr size_t get_tr_src_state_idx_v = get_state_idx_v<get_tr_src_state<TransitionT>, StateSetT>;
  template<typename StateSetT, typename TransitionT>
  inline constexpr size_t get_tr_dest_state_idx_v = get_state_idx_v<get_tr_dest_state<TransitionT>, StateSetT>;

  struct On_entry_invocation {
    template<typename StateT, typename EventT>
    static void invoke(StateT& state, EventT&& evt)
    {
      state.on_entry(std::forward<EventT>(evt));
    }
  };
   
  template<typename StateDataT, typename EventT>
  constexpr void invoke_substate_entry(StateDataT& state_data, EventT&& evt)
  {
    utils::variant_invocation<On_entry_invocation, StateDataT>::invoke(
      state_data, std::forward<EventT>(evt));
  }
  
  struct On_exit_invocation {
    template<typename StateT, typename EventT>
    static void invoke(StateT& state, EventT&& evt)
    {
      state.on_exit(std::forward<EventT>(evt));
    }
  };
  
  template<typename StateDataT, typename EventT>
  constexpr void invoke_substate_exit(StateDataT& state_data, EventT&& evt)
  {
    utils::variant_invocation<On_exit_invocation, StateDataT>::invoke(
      state_data, std::forward<EventT>(evt));
  }
  
  struct Recur_private_on_exit_invocation {
    template<typename StateT, typename EventT>
    static void invoke(StateT& state, EventT&& evt)
    {
      state.private_invoke_on_exit(std::forward<EventT>(evt));
    }
  };
  
  template<typename StateDataT, typename EventT>
  constexpr void invoke_private_exit_recur(StateDataT& state_data, EventT&& evt)
  {
    utils::variant_invocation<Recur_private_on_exit_invocation, StateDataT>::invoke(
      state_data, std::forward<EventT>(evt));
  }
  
  struct Initialize_invocation {
    template<typename StateT, typename EventT>
    static void invoke(StateT& state, EventT&& evt)
    {
      state.initialize(std::forward<EventT>(evt));
    }
  };
  
  template<typename StateDataT, typename EventT>
  constexpr void initialize_substate(StateDataT& state_data, EventT&& evt)
  {
    utils::variant_invocation<Initialize_invocation, StateDataT>::invoke(
      state_data, std::forward<EventT>(evt));
  }
    
  constexpr auto invalid_state_idx_ = std::numeric_limits<size_t>::max();
  
  template<typename StateSetT, typename EventT, typename TransitionT, typename... TransitionTs>
  constexpr auto search_next_state_impl(size_t current_state_idx, EventT&& evt)
  {
    if (get_tr_src_state_idx_v<StateSetT, TransitionT> == current_state_idx &&
      std::is_same_v<get_tr_event<TransitionT>, EventT>) {

      get_tr_action<TransitionT> action;
      action(std::forward<EventT>(evt));
        
      return get_tr_dest_state_idx_v<StateSetT, TransitionT>;
    }
    
    if constexpr (sizeof...(TransitionTs) > 0) {
      return search_next_state_impl<StateSetT, EventT, TransitionTs...>(current_state_idx, std::forward<EventT>(evt));
    } else {
      return invalid_state_idx_;
    }
  }
  
  template<typename StateSetT, typename EventT, typename TransitionTableT>
  struct Next_state_search;
  template<typename StateSetT, typename EventT, typename HeadTrT, typename... TailTrTs>
  struct Next_state_search<StateSetT, EventT, uhsm::Transition_table<HeadTrT, TailTrTs...>> {
    static constexpr auto invalid_state_idx = helpers::invalid_state_idx_;
    
    static constexpr auto search(size_t current_state_idx, EventT&& evt)
    {
      return search_next_state_impl<StateSetT, EventT, HeadTrT, TailTrTs...>(current_state_idx, std::forward<EventT>(evt));
    }
  };
  
  template<typename ActionT>
  struct Action_invocation {
    template<typename SrcStateT, typename EventT>
    static void invoke(const SrcStateT& src_state, EventT&& evt)
    {
      ActionT action;
      action(src_state, std::forward<EventT>(evt));
    }
  };
  
  template<typename ActionT, typename StateDataT, typename EventT>
  constexpr void invoke_action(const StateDataT& state_data, EventT&& evt)
  {
    utils::variant_invocation<Action_invocation<ActionT>, StateDataT>::invoke(
      state_data, std::forward<EventT>(evt));
  }
  
  template<typename StateT, typename EventT, typename TransitionT, typename... TransitionTs>
  constexpr auto get_next_state_perform_action_impl(const StateT& state, EventT&& evt)
  {
    using Nested_state_set = typename StateT::template State_set<StateT>;
    
    if (get_tr_src_state_idx_v<Nested_state_set, TransitionT> == state.state_data.index() &&
      std::is_same_v<get_tr_event<TransitionT>, EventT>) {        
      invoke_action<get_tr_action<TransitionT>>(state.state_data, std::forward<EventT>(evt));
               
      return get_tr_dest_state_idx_v<Nested_state_set, TransitionT>;
    }
    
    if constexpr (sizeof...(TransitionTs) > 0) {
      return get_next_state_perform_action_impl<StateT, EventT, TransitionTs...>(
        state, std::forward<EventT>(evt));
    } else {
      return invalid_state_idx_;
    }
  }
  
  template<typename StateT, typename EventT, typename TransitionTableT>
  struct Next_state_helper;
  template<typename StateT, typename EventT, typename HeadTrT, typename... TailTrTs>
  struct Next_state_helper<StateT, EventT, uhsm::Transition_table<HeadTrT, TailTrTs...>> {
    static constexpr auto invalid_state_idx = helpers::invalid_state_idx_;
    static constexpr auto get_next_state_perform_action(const StateT& state, EventT&& evt)
    {
      return get_next_state_perform_action_impl<StateT, EventT, HeadTrT, TailTrTs...>(
        state, std::forward<EventT>(evt));
    }
  };
  
  // TODO: rename `Next_state_search` `and search_next_state_impl` to indicate
  // that they call transition actions
  
  template<typename StateT, typename EventT, typename NestedStateT, typename... NestedStateTs>
  constexpr auto dispatch_event_impl(StateT& state, EventT&& evt) {
    using Nested_state_set = typename StateT::template State_set<StateT>;
      
    if (constexpr auto state_idx = get_state_idx_v<NestedStateT, Nested_state_set>;
      state_idx == state.state_data.index()) {
      // current state for this state hierarchy level found; dispatch the event to it by calling `react()`
      auto& current_nested_state = std::get<state_idx>(state.state_data);
          
      if (current_nested_state.react(std::forward<EventT>(evt))) {
        // the event was handled; end of processing
        return true;
      }
          
      // NOTE: the event could not be handled at more nested hierarchy level;
      // try to handle it at this level
          
      const auto next_state_idx = Next_state_helper<StateT, EventT, typename StateT::Transitions>
        ::get_next_state_perform_action(state, std::forward<EventT>(evt));
          
      if (next_state_idx == state.state_data.index()) {
        // this is an internal transition (source state and destination state are the same);
        // do not change the state data in any way
        return true;
      }
        
      if (next_state_idx == invalid_state_idx_) {
        // the event cannot be handled at this level (no matching entry in the transition table);
        // defer processing of the event to a higher hierarchy level
        return false;
      }
      
      // NOTE: at this point it is known that at this level a state branch switch occurs
        
      // recursively call on_exit on all current nested states (in LIFO order)
      // before switching to a new state branch
      invoke_private_exit_recur(state.state_data, std::forward<EventT>(evt));
        
      state.state_data = utils::Variant_by_index<Nested_state_set>::make(next_state_idx);
      invoke_substate_entry(state.state_data, std::forward<EventT>(evt));    
      // recursively set initial state for the new current substate
      initialize_substate(state.state_data, std::forward<EventT>(evt));
        
      return true;
    }
      
    if constexpr (sizeof...(NestedStateTs) > 0) {
      return dispatch_event_impl<StateT, EventT, NestedStateTs...>(state, std::forward<EventT>(evt));
    } else {   
      // WARNING: cannot reach here; every state hierarchy level must have a current state at any given time
      // TODO: implement exception handling
      return false;
    }
  }
  
  template<typename StateT, typename EventT, typename NestedStateSetT>
  struct Event_dispatcher;
  template<typename StateT, typename EventT, typename NestedStateT, typename... NestedStateTs>
  struct Event_dispatcher<StateT, EventT, std::tuple<NestedStateT, NestedStateTs...>> {
    static constexpr auto dispatch(StateT& state, EventT && event) {
      return dispatch_event_impl<StateT, EventT, NestedStateT, NestedStateTs...>(
        state, std::forward<EventT>(event));
    }
  };
    
  template<typename TransitionTableT>
  using get_state_data_def_t = utils::apply_func_t<std::variant, helpers::extract_state_set_t<TransitionTableT>>;
  
  // Compile-time tests
  ////////////////////////////////////////////////////////////////////////////////
  
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
  
  namespace Test::HasTrWSrcState_DoesNotContain_ReturnFalse
  {
    struct Unknown_state {};
    using Tested_tr = uhsm::Transition<Unknown_state, Test_data::Pwr_btn_pressed, Test_data::On>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Test_data::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::false_type>);
  }
  
  namespace Test::HasTrWSrcState_DoesContain_ReturnTrue
  {
    struct Unknown_state {};
    // NOTE: it does not matter that the transition table does not have a transition
    // exactly like the following one; only matching source types matters
    using Tested_tr = uhsm::Transition<Test_data::Off, Test_data::Pwr_btn_pressed, Unknown_state>;
    using Bool_contains = has_tr_w_src_state_t<Tested_tr, Test_data::Transitions>;
    static_assert(std::is_same_v <Bool_contains, std::true_type>);
  }
  
  namespace Test::AddUniqueTrWSrcState_AddAlreadyPresent_TableUnchanged
  {
    using New_transition = uhsm::Transition<Test_data::Off, Test_data::Brownout, Test_data::Off>;
    using New_table = add_unique_tr_w_src_state_t<New_transition, Test_data::Transitions>;
    // NOTE: new transition not added to the table because there is already a transition
    // with 'Off' source state
    static_assert(std::is_same_v<New_table, Test_data::Transitions>);
  }
  
  namespace Test::AddUniqueTrWSrcState_AddNotPresent_TransitionAdded
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
  
  namespace Test::RmDuplTrBySrc_NoDupl_TableUnchanged
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
  
  namespace Test::RmDuplTrBySrc_DuplPresent_DuplRemoved
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
  
  namespace Test::StateData_TransitionTablePassed_ReturnStateData
  {
    struct StateA {};
    struct StateB {};
    struct StateC {};
    struct Event1 {};
    struct Event2 {};
    using Transitions = uhsm::Transition_table<
      uhsm::Transition<StateA, Event1, StateB>,
      uhsm::Transition<StateB, Event2, StateA>,
      uhsm::Transition<StateC, Event1, StateA>
    >;
    using State_data_def = get_state_data_def_t<Transitions>;
    static_assert(std::is_same_v<State_data_def, std::variant<StateA, StateB, StateC>>);
  }
}

#endif
