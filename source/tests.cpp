#include <iostream>

#include <cfepi/modeling.h>
#include <cfepi/config.h>
#include <cfepi/sample_view.h>

namespace detail {
/*
constexpr std::string_view json_config = "{    \"states\": [\"S\", \"I\", \"R\", \"V\"],    \"events\":    [	{	    \"source\": [ [\"I\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"R\"} ],	    \"comment_rate\": \"1 / 2.25 = 0.4444444444444444\",	    \"rate\": 0.4444444444444444	},	{	    \"source\": [ [ \"I\" ], [ \"S\", \"V\" ] ],	    \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} ],	    \"comment_rate\": \"1.75 * 1 / 2.25 / 400000 = 0.000001944444\",	    \"rate\": 0.000001944444	}    ],    \"initial_conditions\": {	\"S\": 399995,	\"I\": 5    },    \"worlds\": [	{	    \"event_filter\": {\"function\": \"do_nothing\"},	    \"state_modifier\": {\"function\": \"do_nothing\"},	    \"state_filter\": {\"function\": \"do_nothing\"}	},	{	    \"event_filter\": {		\"function\": \"flat_reduction_single_compartment\",		\"parameters\" : {		    \"event_index\": 1,		    \"reduction_percentage\": 0.05,		    \"compartment_to_filter\": \"S\",		    \"index_to_filter\": 1		}	    },	    \"state_modifier\": {\"function\": \"do_nothing\"},	    \"state_filter\": {\"function\": \"do_nothing\"}	},	{	    \"event_filter\": {		\"function\": \"flat_reduction_single_compartment\",		\"parameters\" : {		    \"event_index\": 1,		    \"reduction_percentage\": 1.0,		    \"compartment_to_filter\": \"V\",		    \"index_to_filter\": 1		}	    },	    \"state_modifier\": {		\"function\": \"single_time_move\",		\"parameters\" : {		    \"percent_to_move\": 0.033,		    \"source_compartments\": [\"S\"],		    \"destination_compartment\": \"V\",		    \"time\": 0		}	    },	    \"state_filter\": {\"function\": \"do_nothing\"}	}    ]}";
constexpr auto states_size = daw::json::parse_json_array_size<std::string_view>(
  daw::json::parse_json_select(json_config, "states"));
constexpr auto states_arr = daw::json::parse_json_array_values<std::string_view, states_size>(
  daw::json::parse_json_select(json_config, "states"));
constexpr cfepi::config_epidemic_states<states_size> states(states_arr);
constexpr auto num_event_types =
  daw::json::parse_json_array_size<daw::json::json_delayed<daw::json::no_name, std::string_view>>(
    daw::json::parse_json_select(json_config, "events"));
constexpr std::array<size_t, num_event_types> event_sizes =
  daw::json::parse_json_array_event_type_sizes<decltype(states), num_event_types>(
    daw::json::parse_json_select(json_config, "events"));
typedef daw::json::parse_json_array_event_type_struct<decltype(states), event_sizes>::any_event_type
  any_config_event_type;
typedef cfepi::any_event<any_config_event_type>::type any_config_event;


*/
}// namespace detail

using namespace detail;

int main(int argc [[maybe_unused]], char **argv [[maybe_unused]]) {

/*
  if (argc != 4) {
    std::cout << "This program takes 3 arguments: population size, number of time steps and seed."
              << std::endl;
    return 1;
  }
  size_t seed = static_cast<size_t>(atoi(argv[3]));
  const cfepi::person_t population_size = static_cast<cfepi::person_t>(atoi(argv[1]));
  const cfepi::epidemic_time_t epidemic_time = static_cast<cfepi::epidemic_time_t>(atof(argv[2]));
  std::random_device rd;
  std::default_random_engine random_source_1{ seed++ };

  auto always_true_event = [](const auto &param __attribute__((unused)),
                             const auto &state __attribute__((unused)),
                             std::default_random_engine &rng
                             __attribute__((unused))) { return (true); };
  auto always_true_state = [](const auto &first_param __attribute__((unused)),
                             const auto &second_param __attribute__((unused)),
                             std::default_random_engine &rng
                             __attribute__((unused))) { return (true); };
  auto always_false = [](const auto &param __attribute__((unused)),
                        std::default_random_engine &rng
                        __attribute__((unused))) { return (false); };
  auto sometimes_true = [](const any_config_event &event,
                          const cfepi::sir_state<decltype(states)> &state __attribute__((unused)),
                          std::default_random_engine &rng) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    if (std::holds_alternative<std::variant_alternative_t<1, any_config_event>>(event)
      // std::holds_alternative<infection_event>(event)
    ) {
      if (dist(rng) < .1) { return (false); }
    }
    return (true);
  };
  auto do_nothing = [](auto &param __attribute__((unused)),
                      std::default_random_engine &rng __attribute__((unused))) { return; };

  double gamma = 1 / 2.25;
  double beta = 1.75 * gamma / static_cast<double>(population_size);
  std::cout << "gamma is " << gamma << "\nbeta is " << beta << "\n";
  auto initial_conditions =
    cfepi::default_state<decltype(states)>(states["S"], states["I"], population_size, 1UL);

  cfepi::
      run_simulation<decltype(states), any_config_event_type, any_config_event>(
          initial_conditions,
          std::array<double, 2>(
              {.1, 2. / static_cast<double>(population_size)}),
          {
              std::make_tuple(always_true_event, always_true_state, do_nothing),
              std::make_tuple(sometimes_true, always_true_state, do_nothing)
              // std::make_tuple(always_false, always_true, do_nothing),
          },
          epidemic_time,
          seed);

  constexpr auto event_type_tuple =
    daw::json::parse_json_array_event_type_struct<decltype(states), event_sizes>{}.parse(
      states, daw::json::parse_json_select(json_config, "events"));

  auto simulation =
    cfepi::run_simulation<decltype(states), any_config_event_type, any_config_event>(event_type_tuple,
      initial_conditions,
      std::array<double, 2>({ .1, 2. / static_cast<double>(population_size) }),
      {
        std::make_tuple(always_true_event, always_true_state, do_nothing),
        std::make_tuple(sometimes_true, always_true_state, do_nothing)
        // std::make_tuple(always_false, always_true, do_nothing),
      },
      epidemic_time,
      seed);

  for (auto state_group : simulation) {
    size_t state_group_index{ 0 };
    for (auto state : state_group) {
      cfepi::print(state, "group " + std::to_string(state_group_index) + " : ");
      ++state_group_index;
    }
  }
*/
}
