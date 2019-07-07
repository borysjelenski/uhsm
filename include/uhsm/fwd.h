#ifndef UHSM_FWD_H
#define UHSM_FWD_H

#include <tuple>

namespace uhsm
{
  template<typename... ElemTs>
  using Transition = std::tuple<ElemTs...>;
  template<typename... TransitionsT>
  using Transition_table = std::tuple<TransitionsT...>;
    
  struct Empty_action {
    template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&&) {}
  };
}

#endif
