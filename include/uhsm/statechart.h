#ifndef UHSM_STATECHART_H
#define UHSM_STATECHART_H

#include <cstddef>
#include <tuple>
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
    
    template<typename EventT>
    void react(EventT&& evt)
    {      
      curr_state_idx = helpers::Event_dispatcher<State_set<T>, EventT, Transitions<T>>
        ::dispatch(curr_state_idx, std::forward<EventT>(evt));
    }
    
    size_t curr_state_idx;
  };
  
  struct No_parent_state {};
  
  template<typename T>
  using Statechart = Complex_state<T, No_parent_state>;
}

#endif
