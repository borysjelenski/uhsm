#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "uhsm/state_machine.h"

TEST_GROUP(Action_TestGroup)
{
  void teardown()
  {
    mock().clear();
  }
};

namespace Event
{
  struct Pwr_switch_flip {};
  struct Play_pause_btn {};
  struct Stop_btn {};
  struct Forward_btn {
    Forward_btn(int amount_sec) : amount_sec{amount_sec} {}
    int amount_sec;
  };
  struct Backward_btn {};
}

namespace Action
{
  struct Turn_led_on {
    template<typename SrcStateT, typename EventT>
    void operator()(const SrcStateT&, EventT&& evt)
    {
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

    static constexpr auto expected_sec_amount = 37;

    template<typename SrcStateT>
    void operator()(const SrcStateT&, Event::Forward_btn&& evt)
    {
      mock().actualCall("playback_pos_forward_w_data");
      LONGS_EQUAL(expected_sec_amount, evt.amount_sec);
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

struct Player_w_actions : uhsm::State_machine<Player_w_actions> {
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

TEST(Action_TestGroup, React_CascadeTrs0_InvokeActionHandlers)
{
  Player_w_actions sm;
  sm.start();   // initialized with 'Powered_off'

  mock().strictOrder();
  mock().expectOneCall("turn_led_on");
  mock().expectOneCall("turn_led_off");

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped', invokes 'turn_led_on' action
  sm.react(Event::Pwr_switch_flip{});   // leaves 'Powered_on' substate, enters 'Powered_off',
                                        // invokes 'turn_led_off()' action

  mock().checkExpectations();
}

TEST(Action_TestGroup, React_CascadeTrs1_InvokeActionHandlers)
{
  Player_w_actions sm;
  sm.start();   // initialized with 'Powered_off'
  
  mock().strictOrder();
  mock().expectOneCall("turn_led_on");
  mock().expectOneCall("turn_led_off");
  
  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped', invokes 'turn_led_on' action
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'
  sm.react(Event::Pwr_switch_flip{});   // leaves 'Powered_on' substate, enters 'Powered_off',
                                        // invokes 'turn_led_off()' action

  mock().checkExpectations();
}

TEST(Action_TestGroup, React_InternalTr0_InvokeActionHandlers)
{
  Player_w_actions sm;
  sm.start();   // initialized with 'Powered_off'
  
  mock().strictOrder();
  mock().expectOneCall("turn_led_on");
  mock().expectOneCall("playback_pos_backward");

  sm.react(Event::Pwr_switch_flip{}); // enters 'Powered_on::Stopped', invokes 'turn_led_on' action
  sm.react(Event::Play_pause_btn{});  // enters 'Powered_on::Active::Playing'
  sm.react(Event::Backward_btn{});    // defers processing to 'Powered_on::Active' substate, remains in 'Playing' state,
                                      // invokes 'playback_pos_backward()' action

  mock().checkExpectations();
}

TEST(Action_TestGroup, React_InternalTrWithEventData0_InvokeActionHandlersPassData)
{
  Player_w_actions sm;
  sm.start();   // initialized with 'Powered_off'
  
  mock().strictOrder();
  mock().expectOneCall("turn_led_on");
  mock().expectOneCall("playback_pos_forward_w_data");
  
  sm.react(Event::Pwr_switch_flip{});       // enters 'Powered_on::Stopped', invokes 'turn_led_on' action
  sm.react(Event::Play_pause_btn{});        // enters 'Powered_on::Active::Playing'
  constexpr auto evt_data = Action::Playback_pos_forward::expected_sec_amount;
  sm.react(Event::Forward_btn{evt_data});   // defers processing to 'Powered_on::Active' substate, remains in 'Playing' state,
                                            // invokes 'playback_pos_forward()' action

  mock().checkExpectations();
}
