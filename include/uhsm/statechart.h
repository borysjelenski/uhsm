#ifndef UHSM_STATECHART_H
#define UHSM_STATECHART_H

#include <cassert>
#include <cstddef>
#include <tuple>
#include <variant>
#include "uhsm/fwd.h"
#include "uhsm/utils.h"
#include "uhsm/helpers.h"

namespace uhsm
{ 
  template<typename T>
  struct Derived_traits {
    using Initial = typename T::Initial;
    using Transitions = typename T::Transitions;
  };
  
  template<typename T, typename ParentStateT>
  struct Simple_state {
    using Parent = ParentStateT;
    
    void on_entry() {}
    void on_exit() {}
    
    void initialize()
    {
      // NOTE: nothing to be initialized in a simple state
      return;
    }
    
    template<typename EventT>
    bool react(EventT&& evt)
    {
      // NOTE: an event can never be handled within a simple state
      return false;
    }
  };
  
  template<typename T, typename ParentStateT>
  struct Complex_state {
    using Parent = ParentStateT;
    
    template<typename U>
    using Simple_state = uhsm::Simple_state<U, Complex_state<T, Parent>>; 
    template<typename U>
    using State = uhsm::Complex_state<U, Complex_state<T, Parent>>;
    template<typename SrcStateT, typename EventT, typename DestStateT,
      typename ActionT = uhsm::Empty_action>
    using Transition = uhsm::Transition<SrcStateT, EventT, DestStateT, ActionT>;
    template<typename... TransitionsT>
    using Transition_table = uhsm::Transition_table<TransitionsT...>;
    template<typename TransitionTableT>
    using State_data_def = helpers::get_state_data_def_t<TransitionTableT>;
    
    template<typename U>
    using State_set = helpers::extract_state_set_t<typename Derived_traits<U>::Transitions>;
    template<typename U>
    using Transitions = typename Derived_traits<U>::Transitions;
    template<typename U>
    using Initial = typename Derived_traits<U>::Initial;
    
    void on_entry() {}
    void on_exit() {}
    
    void initialize()
    {
      // WARNING: this member function MUST be called by the user on topmost
      // state machine object BEFORE any events are passed to it;
      // it recursively constructs objects of initial state for each nesting level;
      // NOTE: this violates RAII but even in case of exception being thrown
      // there would be no memory leak as all data required by
      // the state machine has direct storage;
      // TODO: add a flag indicating whether the complex state has been initialized
      // check it when processing an event and raise an exception if 
      
      auto& state_data = (static_cast<T&>(*this)).state_data;
      state_data = Initial<T>{};
      
      auto& current_state = std::get<Initial<T>>(state_data);
      current_state.on_entry();
      current_state.initialize();
    }
        
    template<typename EventT>
    bool react(EventT&& evt)
    {
      auto& derived = static_cast<T&>(*this); 
      const bool handled = helpers::Event_dispatcher<T, EventT, State_set<T>>::dispatch(
        derived, std::forward<EventT>(evt));
      
      return handled;
    }
  };
  
  struct No_parent_state {};
  
  template<typename T>
  using Statechart = Complex_state<T, No_parent_state>;
}

#endif
