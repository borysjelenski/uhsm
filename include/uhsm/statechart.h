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
    
    void start()
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
    template<typename SrcStateT, typename EventT, typename DestStateT>
    using Transition = uhsm::Transition<SrcStateT, EventT, DestStateT>;
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
    
    Complex_state()
      : curr_state_idx{helpers::get_state_idx_v<Initial<T>, State_set<T>>}
    {
    }
    
    void start()
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
      
      constexpr auto initial_state_idx = helpers::get_state_idx_v<Initial<T>, State_set<T>>;
      assert(state_data.index() == initial_state_idx);
      std::get<initial_state_idx>(state_data).start();
    }
    
    template<typename EventT>
    bool react(EventT&& evt)
    {
      T& derived = static_cast<T&>(*this);
      
      const auto new_state_idx = helpers::Event_dispatcher<State_set<T>, EventT, Transitions<T>>
        ::dispatch(curr_state_idx, std::forward<EventT>(evt));
      
      if (new_state_idx != std::numeric_limits<size_t>::max()) {
        curr_state_idx = new_state_idx;
        
        return true;
      }
      
       
    }
    
    size_t curr_state_idx;
    
  };
  
  struct No_parent_state {};
  
  template<typename T>
  using Statechart = Complex_state<T, No_parent_state>;
}

#endif
