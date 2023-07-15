#include <doctest/doctest.h>
#include <type_traits>

#include <cfepi/sir.h>
#include <cfepi/config.h>

constexpr size_t false_const = 55UL;

TEST_CASE("[repeat] Repeat works") {
  static_assert(std::is_same<cfepi::detail::repeat<int, 1>, int>::value);
  static_assert(std::is_same<cfepi::detail::repeat<int, 2>, int>::value);
  static_assert(std::is_same<cfepi::detail::repeat<int, 3>, int>::value);
}

TEST_CASE("[sir_state] Sized Enum concept works") {

  struct simple_enum {
  public:
    enum state { A, B, C, D, E, state_count };
    constexpr static auto size() { return (static_cast<size_t>(state_count)); }
  };

  constexpr bool simple_enum_is_sized_enum = cfepi::is_sized_enum<simple_enum>;
  static_assert(simple_enum_is_sized_enum);

  auto x = cfepi::sir_state<simple_enum>{ 1UL }.potential_states[0];
  static_assert(std::is_same<decltype(x), std::bitset<simple_enum::state_count>>::value);
  cfepi::sir_state<simple_enum> test_state{ 1UL };
  test_state.potential_states[0][simple_enum::D] = true;
  test_state.potential_states[0][simple_enum::D] = true;
}

struct map_sir_epidemic_states {
public:
  using state = std::string_view;
  constexpr static std::array<std::tuple<std::string_view, size_t>, 3> state_array{
    std::make_tuple("S", 0UL),
    std::make_tuple("I", 1UL),
    std::make_tuple("R", 2UL)
  };
  constexpr static auto size() { return (std::size(state_array)); };
  constexpr size_t operator[](std::string_view s) {
    return (
      std::get<1>(*std::find_if(std::begin(state_array), std::end(state_array), [&s](auto &x) {
        return (std::get<0>(x) == s);
      })));
  }
};

TEST_CASE("[sir_state] map_sir_epidemic_states gives sizes when accessed by string") {
  map_sir_epidemic_states s{};
  static_assert(s["S"] == 0);
  static_assert(s["I"] == 1);
  static_assert(s["R"] == 2);
  static_assert(std::size(s) == 3UL);
  std::bitset<std::size(s)>{ 1UL << s["I"] };
  constexpr bool map_sir_epidemic_states_is_sized_enum =
    cfepi::is_sized_enum<map_sir_epidemic_states>;
  static_assert(map_sir_epidemic_states_is_sized_enum);
}

struct map_sir_recovery_event_type : public cfepi::transition_event_type<map_sir_epidemic_states> {
  constexpr map_sir_recovery_event_type() noexcept
    : transition_event_type<map_sir_epidemic_states>(
      { std::bitset<std::size(map_sir_epidemic_states{})>{
        1UL << map_sir_epidemic_states{}["I"] } },
      "R"){};
};

struct map_sir_infection_event_type
  : public cfepi::interaction_event_type<map_sir_epidemic_states> {
  constexpr map_sir_infection_event_type() noexcept
    : interaction_event_type<map_sir_epidemic_states>(
      { std::bitset<std::size(map_sir_epidemic_states{})>{
        1UL << map_sir_epidemic_states{}["S"] } },
      { std::bitset<std::size(map_sir_epidemic_states{})>{
        1UL << map_sir_epidemic_states{}["I"] } },
      "I"){};
};

typedef std::variant<map_sir_recovery_event_type, map_sir_infection_event_type>
  any_map_sir_event_type;


namespace daw::json {

  /*
TEST_CASE("[config] Basic JSON parser tests") {
  constexpr std::string_view json_config =
    "{    \"name\": \"test_config\",    \"states\": [\"S\", \"I\", \"R\"],    \"events\": [	"
    "{	    \"name\": \"sir_recovery_event\",	    \"type\": \"transition_event\",	    "
    "\"parameters\": {		\"source\": [\"I\"],		\"destination\": \"R\",		"
    "\"rate\": \".1\"	    }	},	{	    \"name\": \"sir_infection_event\",	   "
    " \"type\": \"interaction_event\",	    \"parameters\": {		\"source\": [\"S\"],	"
    "	\"contact\": [\"I\"],		\"destination\": \"I\",		\"rate\": "
    "\".2\",		\"population_dependence\": \"frequency\"	    }	}    ],    "
    "\"parameters\": {	\"population_size\": 10000,	\"initial_infected\": 1    },    "
    "\"worlds\": [	{	    \"event_filter\": { \"function\": \"do_nothing\" }	    "
    "\"state_modifier\": { \"function\": \"do_nothing\" },	    \"state_filter\": { "
    "\"function\": \"do_nothing\" }	},	{	    \"event_filter\": { \"function\": "
    "\"do_nothing\" },	    \"state_modifier\": { \"function\": \"do_nothing\" },	    "
    "\"state_filter\": { \"function\": \"do_nothing\" }	}    ]}";
  constexpr auto t1 = daw::json::from_json<std::string_view>(json_config, "name");
  static_assert(t1 == "test_config");
  try {
    std::string_view test __attribute__((unused)) = std::get<1>(
      daw::json::json_details::find_range<NoCommentSkippingPolicyChecked>(json_config, "states"));
  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for compartment set.") {

  constexpr const std::string_view json_states_member_1 = "[\"S\", \"I\", \"R\"]";
  constexpr auto json_size_1 = parse_json_array_size<std::string_view>(json_states_member_1);
  static_assert(json_size_1 == 3);
  constexpr auto json_vals_1 =
    parse_json_array_values<std::string_view, json_size_1>(json_states_member_1);
  static_assert(json_vals_1[0] == "S");
  static_assert(json_vals_1[1] == "I");
  static_assert(json_vals_1[2] == "R");
  constexpr cfepi::config_epidemic_states<json_size_1> s_1(json_vals_1);
  static_assert(s_1.size() == 3UL);

  constexpr const std::string_view json_states_member_2 = "[\"S\", \"E\", \"I\", \"R\"]";
  constexpr auto json_size_2 = parse_json_array_size<std::string_view>(json_states_member_2);
  static_assert(json_size_2 == 4);
  constexpr auto json_vals_2 =
    parse_json_array_values<std::string_view, json_size_2>(json_states_member_2);
  static_assert(json_vals_2[0] == "S");
  static_assert(json_vals_2[1] == "E");
  static_assert(json_vals_2[2] == "I");
  static_assert(json_vals_2[3] == "R");
  constexpr cfepi::config_epidemic_states<json_size_2> s_2(json_vals_2);
  static_assert(s_2.size() == 4UL);

  constexpr std::string_view json_config_full =
    "{    \"name\": \"test_config\",    \"states\": [\"S\", \"I\", \"R\"],    \"events\": [	"
    "{	    \"name\": \"sir_recovery_event\",	    \"type\": \"transition_event\",	    "
    "\"parameters\": {		\"source\": [\"I\"],		\"destination\": \"R\",		"
    "\"rate\": \".1\"	    }	},	{	    \"name\": \"sir_infection_event\",	   "
    " \"type\": \"interaction_event\",	    \"parameters\": {		\"source\": [\"S\"],	"
    "	\"contact\": [\"I\"],		\"destination\": \"I\",		\"rate\": "
    "\".2\",		\"population_dependence\": \"frequency\"	    }	}    ],    "
    "\"parameters\": {	\"population_size\": 10000,	\"initial_infected\": 1    },    "
    "\"worlds\": [	{	    \"event_filter\": { \"function\": \"do_nothing\" }	    "
    "\"state_modifier\": { \"function\": \"do_nothing\" },	    \"state_filter\": { "
    "\"function\": \"do_nothing\" }	},	{	    \"event_filter\": { \"function\": "
    "\"do_nothing\" },	    \"state_modifier\": { \"function\": \"do_nothing\" },	    "
    "\"state_filter\": { \"function\": \"do_nothing\" }	}    ]}";
  constexpr std::string_view json_states_member_3 =
    std::get<1>(daw::json::json_details::find_range<NoCommentSkippingPolicyChecked>(
      json_config_full, "states"));
  constexpr auto json_size_3 = parse_json_array_size<std::string_view>(json_states_member_3);
  static_assert(json_size_3 == 3);
  constexpr auto json_vals_3 =
    parse_json_array_values<std::string_view, json_size_3>(json_states_member_3);
  static_assert(json_vals_3[0] == "S");
  static_assert(json_vals_3[1] == "I");
  static_assert(json_vals_3[2] == "R");
  constexpr cfepi::config_epidemic_states<json_size_3> s_3(json_vals_3);
  static_assert(s_3.size() == 3UL);

  auto initial_conditions_1 =
    cfepi::default_state<decltype(s_1)>(s_1[json_vals_1[0]], s_1[json_vals_1[1]], 1000UL, 1UL);
  auto initial_conditions_2 =
    cfepi::default_state<decltype(s_2)>(s_2[json_vals_2[0]], s_2[json_vals_2[2]], 1000UL, 1UL);
  auto initial_conditions_3 =
    cfepi::default_state<decltype(s_3)>(s_3[json_vals_3[0]], s_3[json_vals_3[1]], 1000UL, 1UL);
}

TEST_CASE("[config] JSON config parsing works for single event source") {
  constexpr cfepi::config_epidemic_states<3> sir({ "S", "I", "R" });
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  constexpr std::string_view json_config_1{ "[\"I\"]" };
  constexpr std::string_view json_config_2{ "[\"S\", \"V\"]" };
  constexpr auto event_source_1_sir =
    parse_json_event_type_source<decltype(sir)>(sir, json_config_1);
  std::cout << event_source_1_sir << "\n";
  static_assert(event_source_1_sir[0] == false);
  static_assert(event_source_1_sir[1] == true);
  static_assert(event_source_1_sir[2] == false);
  constexpr auto event_source_1_sirv =
    parse_json_event_type_source<decltype(sirv)>(sirv, json_config_1);
  std::cout << event_source_1_sirv << "\n";
  static_assert(event_source_1_sirv[0] == false);
  static_assert(event_source_1_sirv[1] == true);
  static_assert(event_source_1_sirv[2] == false);
  static_assert(event_source_1_sirv[3] == false);
  constexpr auto event_source_2_sirv =
    parse_json_event_type_source<decltype(sirv)>(sirv, json_config_2);
  std::cout << event_source_2_sirv << "\n";
  static_assert(event_source_2_sirv[0] == true);
  static_assert(event_source_2_sirv[1] == false);
  static_assert(event_source_2_sirv[2] == false);
  static_assert(event_source_2_sirv[3] == true);
}

TEST_CASE("[config] JSON config parsing works for multiple event sources") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  constexpr std::string_view json_config{ "[[\"I\"], [\"S\", \"V\"]]" };
  constexpr auto event_source =
    parse_json_event_type_all_sources<decltype(sirv), 2>(sirv, json_config);
  static_assert(event_source[0][0] == false);
  static_assert(event_source[0][1] == true);
  static_assert(event_source[0][2] == false);
  static_assert(event_source[0][3] == false);
  static_assert(event_source[1][0] == true);
  static_assert(event_source[1][1] == false);
  static_assert(event_source[1][2] == false);
  static_assert(event_source[1][3] == true);
}

TEST_CASE("[config] JSON config parsing works for single event destination") {
  constexpr cfepi::config_epidemic_states<3> sir({ "S", "I", "R" });
  constexpr std::string_view json_config_1{ "[ {\"index\": 0, \"compartment\": \"R\"} ]" };
  constexpr std::string_view json_config_2{
    "[ {\"index\": 0, \"compartment\": \"V\"}, {\"index\": 1, "
    "\"compartment\": \"V\"} ]"
  };

  try {
    constexpr auto event_destination_1_1 =
      parse_json_event_type_destination<decltype(sir), 1>(sir, json_config_1);

    static_assert(std::size(event_destination_1_1) == 1);
    static_assert(false_const > std::size(sir));
    static_assert(event_destination_1_1[0].has_value());
    static_assert(event_destination_1_1[0].value_or(false_const) == sir["R"]);

    constexpr auto event_destination_2_1 =
      parse_json_event_type_destination<decltype(sir), 2>(sir, json_config_1);

    static_assert(std::size(event_destination_2_1) == 2);
    static_assert(event_destination_2_1[0].has_value());
    static_assert(event_destination_2_1[0].value_or(false_const) == sir["R"]);
    static_assert(!event_destination_2_1[1].has_value());

    constexpr auto event_destination_2_2 =
      parse_json_event_type_destination<decltype(sir), 2>(sir, json_config_2);

    static_assert(std::size(event_destination_2_2) == 2);
    static_assert(event_destination_2_2[0].has_value());
    static_assert(event_destination_2_2[0].value_or(false_const) == sir["V"]);
    static_assert(event_destination_2_2[1].has_value());
    static_assert(event_destination_2_2[1].value_or(false_const) == sir["V"]);
  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for single event") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  constexpr std::string_view recovery_event_config{
    "{\"source\": [ [\"I\"] ], \"destination\": [ {\"index\": 0, \"compartment\": \"R\"} ]}"
  };
  constexpr std::string_view infection_event_config{
    "{\"source\": [ [ \"I\" ], [ \"S\", \"V\" ] ], \"destination\": [ {\"index\": 1, "
    "\"compartment\": \"I\"} ] }"
  };

  try {
    constexpr auto recovery_event_size = parse_json_event_type_size(recovery_event_config);
    static_assert(recovery_event_size == 1);
    constexpr auto recovery_event_type =
      parse_json_event_type<decltype(sirv), recovery_event_size>(sirv, recovery_event_config);
    static_assert(recovery_event_type.preconditions[0][sirv["S"]] == false);
    static_assert(recovery_event_type.preconditions[0][sirv["I"]] == true);
    static_assert(recovery_event_type.preconditions[0][sirv["R"]] == false);
    static_assert(recovery_event_type.preconditions[0][sirv["V"]] == false);
    static_assert(false_const > std::size(sirv));
    static_assert(recovery_event_type.postconditions[0].has_value());
    static_assert(recovery_event_type.postconditions[0].value_or(false_const) == sirv["R"]);

    constexpr auto infection_event_size = parse_json_event_type_size(infection_event_config);
    static_assert(infection_event_size == 2);
    constexpr auto infection_event_type =
      parse_json_event_type<decltype(sirv), infection_event_size>(sirv, infection_event_config);
    static_assert(infection_event_type.preconditions[0][sirv["S"]] == false);
    static_assert(infection_event_type.preconditions[0][sirv["I"]] == true);
    static_assert(infection_event_type.preconditions[0][sirv["R"]] == false);
    static_assert(infection_event_type.preconditions[0][sirv["V"]] == false);
    static_assert(infection_event_type.preconditions[1][sirv["S"]] == true);
    static_assert(infection_event_type.preconditions[1][sirv["I"]] == false);
    static_assert(infection_event_type.preconditions[1][sirv["R"]] == false);
    static_assert(infection_event_type.preconditions[1][sirv["V"]] == true);
    static_assert(false_const > std::size(sirv));
    static_assert(!infection_event_type.postconditions[0].has_value());
    static_assert(infection_event_type.postconditions[1].has_value());
    static_assert(infection_event_type.postconditions[1].value_or(false_const) == sirv["I"]);
  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for event type tuple.") {
  try {
    constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
    constexpr std::string_view json_config =
      "[{\"source\": [ [\"I\"] ], \"destination\": [ {\"index\": 0, "
      "\"compartment\": \"R\"} ]}, {\"source\": [ [ \"I\" ], [ \"S\", \"V\" "
      "] "
      "], \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} ] }]";

    constexpr auto num_events =
      parse_json_array_size<json_delayed<no_name, std::string_view>>(json_config);
    constexpr std::array<size_t, num_events> event_sizes =
      parse_json_array_event_type_sizes<decltype(sirv), num_events>(json_config);
    print_first_element<event_sizes>();

    constexpr auto event_type_tuple =
      parse_json_array_event_type_struct<decltype(sirv), event_sizes>{}.parse(sirv, json_config);


    constexpr std::array<size_t, 5> test{ 5, 4, 3, 2, 5 };
    print_first_element<test>();

    static_assert(num_events == 2);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][sirv["S"]] == false);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][sirv["I"]] == true);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][sirv["R"]] == false);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][sirv["V"]] == false);
    static_assert(false_const > std::size(sirv));
    static_assert(std::get<0>(event_type_tuple).postconditions[0].has_value());
    static_assert(
      std::get<0>(event_type_tuple).postconditions[0].value_or(false_const) == sirv["R"]);

    static_assert(std::get<1>(event_type_tuple).preconditions[0][sirv["S"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][sirv["I"]] == true);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][sirv["R"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][sirv["V"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][sirv["S"]] == true);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][sirv["I"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][sirv["R"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][sirv["V"]] == true);
    static_assert(false_const > std::size(sirv));
    static_assert(!std::get<1>(event_type_tuple).postconditions[0].has_value());
    static_assert(std::get<1>(event_type_tuple).postconditions[1].has_value());
    static_assert(
      std::get<1>(event_type_tuple).postconditions[1].value_or(false_const) == sirv["I"]);


    typedef parse_json_array_event_type_struct<decltype(sirv), event_sizes>::any_event_type
      any_event;
    static_assert(std::variant_size_v<any_event> == 2);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for event rates.") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  constexpr std::string_view json_config =
    "[{\"source\": [ [\"I\"] ], \"destination\": [ {\"index\": 0, "
    "\"compartment\": \"R\"} ], \"rate\": 0.44444}, {\"source\": [ [ \"I\" ], [ \"S\", "
    "\"V\" "
    "] "
    "], \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} ] , \"rate\": 1.05}]";
  constexpr auto num_events =
    parse_json_array_size<json_delayed<no_name, std::string_view>>(json_config);
  constexpr std::array<double, num_events> event_rates=
    parse_json_array_event_type_rates<double, num_events>(json_config);
  static_assert(event_rates[0] == 0.44444);
  static_assert(event_rates[1] == 1.05);
}

TEST_CASE("[config] JSON config parsing works for initial conditions.") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  constexpr std::string_view json_config = "{\"S\": 9999,\"I\": 1}";
  try {
    auto initial_conditions = parse_json_initial_conditions<decltype(sirv)>(sirv, json_config);
    for(auto i = 0UL; i < 9999UL; ++i){
      CHECK(initial_conditions.potential_states[i][sirv["S"]] == true);
      CHECK(initial_conditions.potential_states[i][sirv["I"]] == false);
      CHECK(initial_conditions.potential_states[i][sirv["R"]] == false);
      CHECK(initial_conditions.potential_states[i][sirv["V"]] == false);
    }
    for(auto i = 9999UL; i < 10000UL; ++i){
      CHECK(initial_conditions.potential_states[i][sirv["S"]] == false);
      CHECK(initial_conditions.potential_states[i][sirv["I"]] == true);
      CHECK(initial_conditions.potential_states[i][sirv["R"]] == false);
      CHECK(initial_conditions.potential_states[i][sirv["V"]] == false);
    }
  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for trivial filtration setup functions.") {
  // TODO : convert this to constexpr if/when possible
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  typedef std::variant<cfepi::sir_event_type<decltype(sirv), 1>, cfepi::sir_event_type<decltype(sirv), 2>>
    sir_events_t;
  auto sample_state{cfepi::default_state<decltype(sirv)>(sirv["S"], sirv["I"], 10000UL, 1UL)};
  std::default_random_engine rng{};
  sir_events_t an_event{};
  cfepi::filtration_setup<decltype(sirv), sir_events_t> a_filtration_setup{
    sample_state,
    trivial_event_filter,
    trivial_state_filter,
    trivial_state_modifier
  };
  constexpr std::string_view json_config =
    "{\"event_filter\": { \"function\": \"do_nothing\" }, \"state_modifier\": { \"function\": "
    "\"do_nothing\" }, \"state_filter\": { \"function\": \"do_nothing\" }}";
  try {
    auto config_event_filter =
      parse_json_event_filter<decltype(sirv), sir_events_t>(parse_json_select(json_config, "event_filter"), sirv);

    CHECK(config_event_filter(an_event, sample_state, rng) == true);

    auto config_state_filter =
      parse_json_state_filter<decltype(sirv), sir_events_t>(parse_json_select(json_config, "state_filter"), sirv);

    CHECK(config_state_filter(a_filtration_setup, sample_state, rng) == true);

    auto config_state_modifier =
      parse_json_state_modifier<decltype(sirv)>(parse_json_select(json_config, "state_modifier"), sirv);

    config_state_modifier(sample_state, rng);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for flat reduction event filter functions.") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  typedef std::variant<cfepi::sir_event_type<decltype(sirv), 1>, cfepi::sir_event_type<decltype(sirv), 2>>
    sir_events_t;
  std::default_random_engine rng{};
  auto sample_state{cfepi::default_state<decltype(sirv)>(sirv["S"], sirv["I"], 10000UL, 1UL)};
  sir_events_t a_recovery_event = cfepi::sir_event_type<decltype(sirv), 1>{};
  sir_events_t an_infection_event = cfepi::sir_event_type<decltype(sirv), 2>{};
  constexpr std::string_view json_config =
    "{\"event_filter\": { \"function\": \"flat_reduction\", \"parameters\" : { \"event_index\": 1, "
    "\"reduction_percentage\": 1.0 } }, \"state_modifier\": { \"function\": "
    "\"do_nothing\" }, \"state_filter\": { \"function\": \"do_nothing\" }}";

  try {
    auto config_event_filter =
      parse_json_event_filter<decltype(sirv), sir_events_t>(parse_json_select(json_config, "event_filter"), sirv);

    CHECK(config_event_filter(a_recovery_event, sample_state, rng) == true);
    CHECK(config_event_filter(an_infection_event, sample_state, rng) == false);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for strict incidence state filter functions.") {
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  typedef std::variant<cfepi::sir_event_type<decltype(sirv), 1>, cfepi::sir_event_type<decltype(sirv), 2>>
    sir_events_t;
  std::default_random_engine rng{};
  auto sample_state{cfepi::default_state<decltype(sirv)>(sirv["S"], sirv["I"], 10000UL, 1UL)};
  cfepi::filtration_setup<decltype(sirv), sir_events_t> a_filtration_setup_low{
    sample_state,
    trivial_event_filter,
    trivial_state_filter,
    trivial_state_modifier
  };
  cfepi::filtration_setup<decltype(sirv), sir_events_t> a_filtration_setup_right{
    sample_state,
    trivial_event_filter,
    trivial_state_filter,
    trivial_state_modifier
  };
  a_filtration_setup_right.states_entered.potential_states[2].flip(sirv["I"]);
  cfepi::filtration_setup<decltype(sirv), sir_events_t> a_filtration_setup_high{
    sample_state,
    trivial_event_filter,
    trivial_state_filter,
    trivial_state_modifier
  };
  a_filtration_setup_high.states_entered.potential_states[2].flip(sirv["I"]);
  a_filtration_setup_high.states_entered.potential_states[3].flip(sirv["I"]);
  constexpr std::string_view json_config =
    "{\"state_filter\": { \"function\": \"strict_incidence_filter\", \"parameters\" : { "
    "\"compartment\" : \"I\", \"counts\": [1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 6, 1, "
    "1, 3, 3, 4, 8, 2, 4, 2, 7, 3, 9, 4, 5, 7, 8, 4, 7, 7, 6, 7, 9, 7, 9, 12, 7, 13, 15, 8, 21, "
    "17, 11, 11, 14, 11, 3, 4, 14, 9, 7, 8] } } }";

    try {
      auto config_state_filter = parse_json_state_filter<decltype(sirv), sir_events_t>(
        parse_json_select(json_config, "state_filter"), sirv);

      CHECK(config_state_filter(a_filtration_setup_low, sample_state, rng) == false);
      CHECK(config_state_filter(a_filtration_setup_right, sample_state, rng) == true);
      CHECK(config_state_filter(a_filtration_setup_high, sample_state, rng) == false);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for single move state modifier setup functions.") {
  // TODO : convert this to constexpr if/when possible
  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  typedef std::variant<cfepi::sir_event_type<decltype(sirv), 1>, cfepi::sir_event_type<decltype(sirv), 2>>
    sir_events_t;
  auto sample_state{cfepi::default_state<decltype(sirv)>(sirv["S"], sirv["I"], 10000UL, 1UL)};
  sample_state.time = 0;
  std::default_random_engine rng{};
  sir_events_t an_event{};
  cfepi::filtration_setup<decltype(sirv), sir_events_t> a_filtration_setup{
    sample_state,
    trivial_event_filter,
    trivial_state_filter,
    trivial_state_modifier
  };
  constexpr std::string_view json_config =
    "{\"state_modifier\": { \"function\": \"single_time_move\", \"parameters\" : { "
    "\"percent_to_move\": 1.0, \"source_compartments\": [\"S\"], \"destination_compartment\": "
    "\"V\", \"time\": 0 } } }";

  try {

    auto config_state_modifier =
      parse_json_state_modifier<decltype(sirv)>(parse_json_select(json_config, "state_modifier"), sirv);

    config_state_modifier(sample_state, rng);
    CHECK(sample_state.potential_states[3][sirv["S"]] == 0);
    CHECK(sample_state.potential_states[3][sirv["V"]] == 1);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for full world.") {
  constexpr std::string_view json_config =
    " [ { \"event_filter\": { \"function\": \"do_nothing\" } \"state_modifier\": { \"function\": "
    "\"do_nothing\" }, \"state_filter\": { \"function\": \"do_nothing\" } }, { \"event_filter\": { \"function\": "
    "\"flat_reduction\", \"parameters\" : { \"event_index\": 0, \"reduction_percentage\": 1.0 } }, \"state_modifier\": "
    "{ \"function\": \"single_time_move\", \"parameters\" : { \"percent_to_move\": 0.24, \"source_compartments\": "
    "[\"S\"], \"destination_compartment\": \"V\", \"time\": 0 } }, \"state_filter\": { \"function\": "
    "\"strict_incidence_filter\", \"parameters\" : { \"compartment\" : \"I\", \"counts\": [1, 1, 1, 2, 1, 1, 1, 1, 1, "
    "1, 1, 1, 1, 1, 2, 2, 6, 1, 1, 3, 3, 4, 8, 2, 4, 2, 7, 3, 9, 4, 5, 7, 8, 4, 7, 7, 6, 7, 9, 7, 9, 12, 7, 13, 15, 8, "
    "21, 17, 11, 11, 14, 11, 3, 4, 14, 9, 7, 8] } } } ] ";

  constexpr cfepi::config_epidemic_states<4> sirv({ "S", "I", "R", "V" });
  typedef std::variant<cfepi::sir_event_type<decltype(sirv), 1>, cfepi::sir_event_type<decltype(sirv), 2>> sir_events_t;

  auto sample_state{cfepi::default_state<decltype(sirv)>(sirv["S"], sirv["I"], 10000UL, 1UL)};
  sample_state.time = 0;

  try {
    auto config_world = parse_json_filtration_setups<decltype(sirv), sir_events_t>(json_config, sirv);
  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}

TEST_CASE("[config] JSON config parsing works for whole config.") {
  constexpr std::string_view json_config = "{    \"states\": [\"S\", \"I\", \"R\", \"V\"],    \"events\":    [	{	    \"source\": [ [\"I\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"R\"} ],	    \"rate\": 0.444444	},	{	    \"source\": [ [ \"I\" ], [ \"S\", \"V\" ] ],	    \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} ],	    \"comment_rate\": \"1.05 * .44444444444444 / 10000\",	    \"rate\": 0.00004666666667	}    ],    \"initial_conditions\": {	\"S\": 9999,	\"I\": 1    },    \"worlds\": [	{	    \"event_filter\": { \"function\": \"do_nothing\" },	    \"state_modifier\": { \"function\": \"do_nothing\" },	    \"state_filter\": {		\"function\": \"strict_incidence_filter\",		\"parameters\" : {		    \"compartment\" : \"I\",		    \"counts\": [		       1,		       2		     ]		}	    }	},	{	    \"event_filter\": {		\"function\": \"flat_reduction\",		\"parameters\" : {		    \"event_index\": 0,		    \"reduction_percentage\": 1.0		}	    },	    \"state_modifier\": {		\"function\": \"single_time_move\",		\"parameters\" : {		    \"percent_to_move\": 0.24,		    \"source_compartments\": [\"S\"],		    \"destination_compartment\": \"V\",		    \"time\": 0		}	    },	    \"state_filter\": { \"function\": \"do_nothing\" }	}    ]}";
  try {

    std::cout << json_config << "\n";
    constexpr auto states_size =
      parse_json_array_size<std::string_view>(parse_json_select(json_config, "states"));

    static_assert(states_size == 4);

    constexpr auto states_arr = parse_json_array_values<std::string_view, states_size>(
      parse_json_select(json_config, "states"));

    static_assert(states_arr[0] == "S");
    static_assert(states_arr[1] == "I");
    static_assert(states_arr[2] == "R");
    static_assert(states_arr[3] == "V");

    constexpr cfepi::config_epidemic_states<states_size> states(states_arr);

    static_assert(states["S"] == 0);
    static_assert(states["I"] == 1);
    static_assert(states["R"] == 2);
    static_assert(states["V"] == 3);

    constexpr auto num_events = parse_json_array_size<json_delayed<no_name, std::string_view>>(
      parse_json_select(json_config, "events"));

    static_assert(num_events == 2);

    constexpr std::array<size_t, num_events> event_sizes =
      parse_json_array_event_type_sizes<decltype(states), num_events>(
        parse_json_select(json_config, "events"));

    static_assert(std::size(event_sizes) == num_events);
    static_assert(event_sizes[0] == 1);
    static_assert(event_sizes[1] == 2);

    typedef parse_json_array_event_type_struct<decltype(states), event_sizes>::any_event_type
      any_event_t;
    constexpr auto event_type_tuple =
      parse_json_array_event_type_struct<decltype(states), event_sizes>{}.parse(
        states, parse_json_select(json_config, "events"));

    static_assert(std::get<0>(event_type_tuple).preconditions[0][states["S"]] == false);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][states["I"]] == true);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][states["R"]] == false);
    static_assert(std::get<0>(event_type_tuple).preconditions[0][states["V"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][states["S"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][states["I"]] == true);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][states["R"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[0][states["V"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][states["S"]] == true);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][states["I"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][states["R"]] == false);
    static_assert(std::get<1>(event_type_tuple).preconditions[1][states["V"]] == true);
    static_assert(false_const > std::size(states));
    static_assert(!std::get<1>(event_type_tuple).postconditions[0].has_value());
    static_assert(std::get<1>(event_type_tuple).postconditions[1].has_value());
    static_assert(
      std::get<1>(event_type_tuple).postconditions[1].value_or(false_const) == states["I"]);

    auto initial_conditions = parse_json_initial_conditions<decltype(states)>(states, parse_json_select(json_config, "initial_conditions"));

    for(auto i = 0UL; i < 9999UL; ++i){
      CHECK(initial_conditions.potential_states[i][states["S"]] == true);
      CHECK(initial_conditions.potential_states[i][states["I"]] == false);
      CHECK(initial_conditions.potential_states[i][states["R"]] == false);
      CHECK(initial_conditions.potential_states[i][states["V"]] == false);
    }
    for(auto i = 9999UL; i < 10000UL; ++i){
      CHECK(initial_conditions.potential_states[i][states["S"]] == false);
      CHECK(initial_conditions.potential_states[i][states["I"]] == true);
      CHECK(initial_conditions.potential_states[i][states["R"]] == false);
      CHECK(initial_conditions.potential_states[i][states["V"]] == false);
    }


    constexpr std::array<double, num_events> event_rates=
      parse_json_array_event_type_rates<double, num_events>(parse_json_select(json_config, "events"));

    static_assert(event_rates[0] == 0.444444);
    static_assert(fabs(event_rates[1] - (1.05 * .44444444444444 / 10000)) < 0.00000000000001 );

    auto config_world = parse_json_filtration_setups<decltype(states), any_event_t>(parse_json_select(json_config, "worlds"), states);

  } catch (daw::json::json_exception const &jex) {
    std::cerr << "Exception thrown by parser: " << jex.reason() << std::endl;
    exit(1);
  }
}
*/

}// namespace daw::json
