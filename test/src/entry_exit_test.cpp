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

struct Player_w_entry_exit : uhsm::State_machine<Player_w_entry_exit> {
  struct Powered_off : Simple_state<Powered_off> {
    template<typename EventT>
    void on_entry(EventT&& evt) { mock().actualCall("powered_off_ENTRY"); }
    template<typename EventT>
    void on_exit(EventT&& evt) { mock().actualCall("powered_off_EXIT"); }
  };

  struct Powered_on : Substate_machine<Powered_on> {
    template<typename EventT>
    void on_entry(EventT&& evt) { mock().actualCall("powered_on_ENTRY"); }
    template<typename EventT>
    void on_exit(EventT&& evt) { mock().actualCall("powered_on_EXIT"); }

    struct Stopped : Simple_state<Stopped> {
      template<typename EventT>
      void on_entry(EventT&& evt) { mock().actualCall("stopped_ENTRY"); }
      template<typename EventT>
      void on_exit(EventT&& evt) { mock().actualCall("stopped_EXIT"); }
    };

    struct Active : Substate_machine<Active> {
      template<typename EventT>
      void on_entry(EventT&& evt) { mock().actualCall("active_ENTRY"); }
      template<typename EventT>
      void on_exit(EventT&& evt) { mock().actualCall("active_EXIT"); }

      struct Playing : Simple_state<Playing> {
        template<typename EventT>
        void on_entry(EventT&& evt) { mock().actualCall("playing_ENTRY"); }
        template<typename EventT>
        void on_exit(EventT&& evt) { mock().actualCall("playing_EXIT"); }
      };

      struct Paused : Simple_state<Paused> {
        template<typename EventT>
        void on_entry(EventT&& evt) { mock().actualCall("paused_ENTRY"); }
        template<typename EventT>
        void on_exit(EventT&& evt) { mock().actualCall("paused_EXIT"); }
      };
      
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

TEST_GROUP(EntryExit_TestGroup)
{
  void teardown()
  {
    mock().clear();
  }
};

TEST(EntryExit_TestGroup, Start_SimpleHSM_NoEntryExitCalls)
{
  Player_w_entry_exit sm;

  mock().expectNoCall("powered_off_ENTRY");

  sm.start();

  mock().checkExpectations();
}

TEST(EntryExit_TestGroup, React_CascadeInTr0_InvokeEntryExitInOrder)
{
  Player_w_entry_exit sm;
  sm.start();   // initialized with 'Powered_off'

  mock().strictOrder();
  mock().expectOneCall("powered_off_EXIT");
  mock().expectOneCall("powered_on_ENTRY");
  mock().expectOneCall("stopped_ENTRY");

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'

  mock().checkExpectations();
}

TEST(EntryExit_TestGroup, React_CascadeInTr1_InvokeEntryExitInOrder)
{
  Player_w_entry_exit sm;
  sm.start();   // initialized with 'Powered_off'

  mock().strictOrder();
  
  mock().expectOneCall("powered_off_EXIT");
  mock().expectOneCall("powered_on_ENTRY");
  mock().expectOneCall("stopped_ENTRY");

  mock().expectOneCall("stopped_EXIT");
  mock().expectOneCall("active_ENTRY");
  mock().expectOneCall("playing_ENTRY");

  sm.react(Event::Pwr_switch_flip{});   // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});    // enters 'Powered_on::Active::Playing'

  mock().checkExpectations();
}

TEST(EntryExit_TestGroup, React_InternalCascadeOutTr0_InvokeEntryExitInOrder)
{
  Player_w_entry_exit sm;
  sm.start();   // initialized with 'Powered_off'

  mock().strictOrder();

  mock().expectOneCall("powered_off_EXIT");
  mock().expectOneCall("powered_on_ENTRY");
  mock().expectOneCall("stopped_ENTRY");

  mock().expectOneCall("stopped_EXIT");
  mock().expectOneCall("active_ENTRY");
  mock().expectOneCall("playing_ENTRY");

  sm.react(Event::Pwr_switch_flip{}); // enters 'Powered_on::Stopped'
  sm.react(Event::Play_pause_btn{});  // enters 'Powered_on::Active::Playing'
  sm.react(Event::Backward_btn{});    // defers processing to 'Powered_on::Active' substate,
                                      // remains in 'Powered_on::Active::Playing'

  mock().checkExpectations();
}
