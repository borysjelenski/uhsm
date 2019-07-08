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

TEST_GROUP(Transition_TestGroup)
{
};

TEST(Transition_TestGroup, Start_SimpleHSM_InExpectedState)
{
  Player sm;
  sm.start();

  CHECK(std::holds_alternative<Player::Powered_off>(sm.state_data));
}

TEST(Transition_TestGroup, React_CascadeInTr0_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Stopped>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_CascadeInTr1_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Active,
  Player::Powered_on::Active::Playing>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_SimpleTr0_InExpectedState)
{
  Player sm;
  sm.start();     // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Paused'

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Active,
  Player::Powered_on::Active::Paused>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_CascadeOutTr0_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'
  sm.react(Event::Stop_btn{});          // leaves 'Powered_on::Active' substate, enters 'Powered_on::Stopped'

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Stopped>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_CascadeOutTr1_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'
  sm.react(Event::Pwr_switch_flip{});   // leaves 'Powered_on' substate, enters 'Powered_off'

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_off>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_InternalCascadeOutTr0_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});                     // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});                      // enters 'Powered_on::Active::Playing'
  const bool handled = sm.react(Event::Backward_btn{});   // defers processing to 'Powered_on::Active' substate,
                                                          // remains in 'Powered_on::Active::Playing'

  // make sure last event was actually handled and not ignored
  CHECK(handled);

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Active,
  Player::Powered_on::Active::Playing>(sm);
  CHECK(in_state);
}

TEST(Transition_TestGroup, React_InternalCascadeOutTr1_InExpectedState)
{
  Player sm;
  sm.start();   // initialized with 'Off'

  sm.react(Event::Pwr_switch_flip{});                     // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});                      // enters 'Powered_on::Active::Playing'
  sm.react(Event::Play_pause_btn{});                      // enters 'Powered_on::Active::Paused'
  const bool handled = sm.react(Event::Forward_btn{});    // defers processing to 'Powered_on::Active' substate,
                                                          // remains in 'Powered_on::Active::Paused'

  // make sure last event was actually handled and not ignored
  CHECK(handled);

  const bool in_state = uhsm::helpers::is_in_state<
  Player,
  Player::Powered_on,
  Player::Powered_on::Active,
  Player::Powered_on::Active::Paused>(sm);
  CHECK(in_state);
}