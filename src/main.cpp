#include "uhsm/statechart.h"

namespace Event
{
  struct Power_pressed {};
  struct Play_pause_pressed {};
}

struct Player : uhsm::Statechart<Player> {
  struct Off : Simple_state<Off> {};
  struct On : State<On> {
    struct Paused : Simple_state<Paused> {};
    struct Playing : Simple_state<Playing> {
      unsigned int playback_dur;
    };
    
    struct Start_playback {
      template<typename EventT>
      void operator()(EventT&& evt) {}
    };
    
    struct Stop_playback {
      template<typename EventT>
      void operator()(EventT&& evt) {}
    };
    
    using Initial = Paused;
    using Transitions = Transition_table<
      Transition<Paused, Event::Play_pause_pressed, Playing, Start_playback>,
      Transition<Playing, Event::Play_pause_pressed, Paused, Stop_playback>
    >;
    
    State_data_def<Transitions> state_data;
  };
  
  using Initial = Off;
  using Transitions = Transition_table<
    Transition<Off, Event::Power_pressed, On>,
    Transition<On, Event::Power_pressed, Off>
  >;
  
  State_data_def<Transitions> state_data;
};

struct Device : uhsm::Statechart<Device>
{
  // States
  struct Disabled : Simple_state<Disabled> {};
  struct Enabled : Simple_state<Enabled> {};
  // Events
  struct Turn_on {};
  struct Turn_off {};
  
  using Initial = Disabled;
  using Transitions = Transition_table<
    Transition<Disabled, Turn_off, Disabled>,
    Transition<Disabled, Turn_on, Enabled>,
    Transition<Enabled, Turn_on, Enabled>,
    Transition<Enabled, Turn_off, Disabled>
  >;
  
  State_data_def<Transitions> state_data;
};

int main()
{
  Player sc;
  sc.reset();
  sc.react(Event::Power_pressed{});
  sc.react(Event::Play_pause_pressed{});
  sc.react(Event::Play_pause_pressed{});
  sc.react(Event::Play_pause_pressed{});
  sc.react(Event::Power_pressed{});
  sc.react(Event::Power_pressed{});
  sc.react(Event::Power_pressed{});
  
  printf("%d", sc.state_data.index());
  
  Device dev_sc;
  dev_sc.reset();
  dev_sc.react(Device::Turn_off{});
  dev_sc.react(Device::Turn_on{});
  
  printf("%d", dev_sc.state_data.index());
}
 