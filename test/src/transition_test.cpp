#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "uhsm/state_machine.h"

namespace Event
{
  struct Pwr_switch_flip {};
  struct Play_pause_btn {};
  struct Stop_btn {};
  struct Forward_btn {};
  struct Backward_btn {};
}

struct Player : uhsm::State_machine<Player> {
  struct Powered_off : Simple_state<Powered_off> {};
  struct Powered_on : Substate_machine<Powered_on> {
    struct Stopped : Simple_state<Stopped> {};
    struct Active : Substate_machine<Active> {
      struct Playing : Simple_state<Playing> {};
      struct Paused : Simple_state<Paused> {};
      
      using Initial = Playing;
      using Transitions = Transition_table<
        Transition<Playing, Event::Play_pause_btn, Paused>,
        Transition<Paused, Event::Play_pause_btn, Playing>
      >;
      
      State_data_def<Transitions> state_data;
    };
    
    using Initial = Stopped;
    using Transitions = Transition_table<
      Transition<Stopped, Event::Play_pause_btn, Active>,
      Transition<Active, Event::Stop_btn, Stopped>,
      Transition<Active, Event::Forward_btn, Active>,
      Transition<Active, Event::Backward_btn, Active>
    >;
    
    State_data_def<Transitions> state_data;
  };
  
  using Initial = Powered_off;
  using Transitions = Transition_table<
    Transition<Powered_off, Event::Pwr_switch_flip, Powered_on>,
    Transition<Powered_on, Event::Pwr_switch_flip, Powered_off>
  >;
  
  State_data_def<Transitions> state_data;
};
