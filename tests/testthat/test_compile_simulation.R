population = 1e3
initial_infected <- 10
simulation_runner <- cfepi:::compile_simulation(
  states = c("S", "I", "R", "V"),
  initial_conditions = c("S" = population - initial_infected, "I" = initial_infected),
  events = list(
    cfepi:::create_event(
      c("S", "I", "R", "V"),
      list(list("I")),
      list(list("index" = 1, compartment = "R")),
      (4 / 9)
    ),
    cfepi:::create_event(
      c("S", "I", "R", "V"),
      list(list("I"), list("S", "V")),
      list(list("index" = 2, compartment = "I")),
      2.05 * (4 / 9) / population
    )
  ),
  filtration_setups = list(
    list(
	    "event_filter" = "std::function([](const any_event &param __attribute__((unused)),
                               const cfepi::sir_state<states_t> &state __attribute__((unused)),
                               std::default_random_engine &rng __attribute__((unused))) {
      std::cout << \"Here\\n\";
     return (true); })",
    "state_filter" = "std::function([](const cfepi::filtration_setup<states_t, any_event> &first_param __attribute__((unused)),
             const cfepi::sir_state<states_t> &second_param __attribute__((unused)),
             std::default_random_engine &rng __attribute__((unused))) { return (true); })",
    "state_modifier" = "std::function([](cfepi::sir_state<states_t> &param __attribute__((unused)),
                         std::default_random_engine &rng __attribute__((unused))) { return; })"
    ),
    list(
	    "event_filter" = "std::function([](const any_event &param __attribute__((unused)),
                               const cfepi::sir_state<states_t> &state __attribute__((unused)),
                               std::default_random_engine &rng __attribute__((unused))) {
      std::uniform_real_distribution<> dist(0.0, 1.0);
      std::cout << \"Here\\n\";
      if (dist(rng) <= 1.0) { return(false); }
      return true;
    })",
    "state_filter" = "std::function([](const cfepi::filtration_setup<states_t, any_event> &first_param __attribute__((unused)),
             const cfepi::sir_state<states_t> &second_param __attribute__((unused)),
             std::default_random_engine &rng __attribute__((unused))) { return (true); })",
    "state_modifier" = "std::function([](cfepi::sir_state<states_t> &param __attribute__((unused)),
                         std::default_random_engine &rng __attribute__((unused))) { return; })"
    )
  )
)

simulation_runner(35, 3)
