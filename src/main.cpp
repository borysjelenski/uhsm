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
    
    using Initial = Paused;
    using Transitions = Transition_table<
      Transition<Paused, Event::Play_pause_pressed, Playing>,
      Transition<Playing, Event::Play_pause_pressed, Paused>
    >;
  };
  
  using Initial = Off;
  using Transitions = Transition_table<
    Transition<Off, Event::Power_pressed, On>,
    Transition<On, Event::Power_pressed, Off>
  >;
}; 

int main()
{
  Player player_sc;
  player_sc.react(Event::Power_pressed{});
}
