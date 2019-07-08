#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <iostream>
#include "uhsm/state_machine.h"

namespace Event
{
  struct Pwr_switch_flip {};
  struct Play_pause_btn {};
  struct Stop_btn {};
  struct Forward_btn {};
  struct Backward_btn {};
}

namespace Action
{
  struct Turn_led_on {
    template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&& evt)
    {
      std::cout << "Dupa\n";

      mock().actualCall("turn_led_on");
    }
  };

  struct Turn_led_off {
    template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&& evt)
    {
      mock().actualCall("turn_led_off");
    }
  };

  struct Playback_pos_forward {
  template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&& evt)
    {
      mock().actualCall("playback_pos_forward");
    }
  };

  struct Playback_pos_backward {
  template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&& evt)
    {
      mock().actualCall("playback_pos_backward");
    }
  };
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
      Transition<Active, Event::Forward_btn, Active, Action::Playback_pos_forward>,
      Transition<Active, Event::Backward_btn, Active, Action::Playback_pos_backward>
    >;
    
    State_data_def<Transitions> state_data;
  };
  
  using Initial = Powered_off;
  using Transitions = Transition_table<
    Transition<Powered_off, Event::Pwr_switch_flip, Powered_on, Action::Turn_led_on>,
    Transition<Powered_on, Event::Pwr_switch_flip, Powered_off, Action::Turn_led_off>
  >;
  
  State_data_def<Transitions> state_data;
};

TEST_GROUP(Action_TestGroup)
{
  void teardown()
  {
    mock().clear();
  }
};
