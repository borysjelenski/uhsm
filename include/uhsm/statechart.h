#ifndef UHSM_STATECHART_H
#define UHSM_STATECHART_H

#include <cstddef>
#include <tuple>
#include "uhsm/fwd.h"
#include "uhsm/utils.h"
#include "uhsm/helpers.h"

namespace uhsm
{  
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
    
    template<typename EventT>
    void react(const EventT& evt)
    {
      
    }
    
    void init_curr_state();
    
    size_t curr_state_idx;
  };
  
  struct No_parent_state {};
  
  template<typename T>
  using Statechart = Complex_state<T, No_parent_state>;
}

#endif
