#' @export
create_event <- function(states, sources, destinations, rate) {
  return(list(
    "source" = sources,
    "destination" = destinations,
    "rate" = rate
  ))
}

parse_single_event <- function(event, states) {
  num_states <- length(states)
  size <- length(event[["source"]])
  source <- event[["source"]]
  destination <- event[["destination"]]
  source_masks <- paste(paste0("std::bitset<", num_states, ">{std::string{\"", stringi::stri_reverse(apply(sapply(
    event[["source"]],
    function(source) {
      rc <- sapply(states, function(y) {source == y} )
      if (length(dim(rc)) > 1) {
        rc <- apply(rc, 2, max)
      } else {
        rc <- ifelse(rc, 1, 0)
      }
      return(rc)
    }
  ), 2, paste, collapse = "")), "\"}, 0, ", num_states, ", '0', '1'}"), collapse = ", ")
  all_sources <- glue::glue(
    "std::array<std::bitset<{{num_states}}>, {{size}}>{ {{source_masks}} }",
    .open = "{{",
    .close = "}}"
  )
  destination_size <- length(destination)
  destination_vec <- rep(NA, times=size)
  for (elem in destination) {
    destination_vec[elem$index] <- elem$compartment
  }
  destination_masks <- paste(sapply(destination_vec, function(value) {
    if (is.na(value)) {
      return ("std::optional<states_t::state>{}")
    }
    value <- which(states == value) - 1
    return (glue::glue("std::optional<states_t::state>{states_t::state{ {{value}} } }", .open = "{{", .close = "}}"))
  }), collapse = ", ")
  all_destinations <- glue::glue(
    "std::array<std::optional<typename states_t::state>, {{size}}>{ {{destination_masks}} }",
    .open = "{{",
    .close = "}}"
  )
  return(glue::glue("cfepi::sir_event_type<states_t, {{size}}>{ {{all_sources}}, {{all_destinations}} }", .open = "{{", .close = "}}"))
}

parse_states_to_code <- function(states) {
  states_size <- length(states)
  states_str <- paste(states, collapse = "\", \"")
  states_type <- glue::glue("cfepi::config_epidemic_states<{{states_size}}>", .open = "{{", .close = "}}")
  states_array <- glue::glue(
    "constexpr std::array<std::string_view, {{states_size}}> states_array { \"{{states_str}}\" }",
    .open = "{{",
    .close = "}}"
  )
  states <- glue::glue(
    "constexpr {{states_type}} states(states_array)",
    .open = "{{",
    .close = "}}"
  )
  states_t_type <- glue::glue("typedef {{states_type}} states_t", .open= "{{", .close = "}}")
  return(glue::glue("{{states_array}};\n{{states}};\n{{states_t_type}};", .open = "{{", .close = "}}"))
}

parse_events_to_code <- function(events, states) {
  size <- length(events)
  num_events <- length(events)
  event_sizes <- sapply(events, function(x) { length(x$source) })
  event_tuple_types <- paste(
    glue::glue("cfepi::sir_event_type<states_t, {{event_sizes}}>", .open = "{{", .close = "}}"),
    collapse = ", "
  )
  variant_type <- glue::glue("typedef typename std::variant<{{event_tuple_types}}> any_event_type", .open = "{{", .close = "}}")
  tuple_type <- glue::glue("std::tuple<{{event_tuple_types}}>", .open = "{{", .close = "}}")
  event_strings <- paste( sapply(events, parse_single_event, states=states), collapse = ", ")
  event_type_tuple <- glue::glue(
    "constexpr {{tuple_type}} event_type_tuple = std::make_tuple( {{event_strings}} )",
    .open = "{{",
    .close = "}}"
  )
  any_event <- "typedef typename cfepi::any_event<any_event_type>::type any_event;"
  options(scipen = 999)
  event_rates <- paste(sapply(events, function(event){ return(event$rate)}), collapse = ", ")
  event_rates <- glue::glue("constexpr std::array<double, {{size}}> event_rates { {{event_rates}} }", .open = "{{", .close = "}}")
  return(glue::glue("{{variant_type}};\n{{event_type_tuple}};\n{{any_event}};\n{{event_rates}};", .open = "{{", .close = "}}" ))
}

parse_initial_conditions_to_code <- function(initial_conditions, states) {
  size <- length(states)
  initial_conditions <- setNames(initial_conditions[states], states)
  if (any(is.na(initial_conditions))) {
    na_valued_compartments <- names(is.na(initial_conditions)[is.na(initial_conditions)])
    warning(paste("The initial value for compartment", na_valued_compartments, "was not specified, defaulting to 0.\n"))
  }
  initial_conditions[is.na(initial_conditions)] <- 0
  population_size <- sum(initial_conditions)
  cumulative_initial_conditions <- cumsum(c(0, initial_conditions))
  cumulative_initial_conditions_lead <- cumulative_initial_conditions[1:length(initial_conditions)]
  cumulative_initial_conditions_lag <- cumulative_initial_conditions[1 + 1:length(initial_conditions)]
  constructor <- glue::glue("cfepi::sir_state<states_t> initial_conditions { {{population_size}} };", .open = "{{", .close = "}}")
  inserter <- paste(glue::glue("for(size_t i = {{cumulative_initial_conditions_lead}}UL; i < {{cumulative_initial_conditions_lag}}UL; ++i){\n  initial_conditions.potential_states[i] = std::bitset<{{size}}> { (1UL << states[ \"{{states}}\" ]) };\n}", .open = "{{", .close = "}}"), collapse = "\n")
  return(glue::glue("{{constructor}};\n{{inserter}};", .open = "{{", .close = "}}"))
}

parse_single_filtration_setup <- function(event_filter, state_filter, state_modifier, unique_identifier) {
  definition_snippet <- glue::glue(
    "
    const auto event_filter_{{unique_identifier}} = {{event_filter}};
    const auto state_filter_{{unique_identifier}} = {{state_filter}};
    const auto state_modifier_{{unique_identifier}} = {{state_modifier}};
    ",
    .open = "{{",
    .close = "}}"
  )

  tuple_snippet <- glue::glue(
    "std::make_tuple(
      event_filter_{{unique_identifier}},
      state_filter_{{unique_identifier}},
      state_modifier_{{unique_identifier}}
    )",
    .open = "{{",
    .close = "}}"
  )
  return(list(definition = definition_snippet, tuple = tuple_snippet))
}


parse_filtration_setups <- function(filtration_setups) {
  parsed_filtration_setups <- lapply(seq(length(filtration_setups)), function(index) {
    setup <- filtration_setups[[index]]

    parse_single_filtration_setup(setup$event_filter, setup$state_filter, setup$state_modifier, unique_identifier = index)
  })
  definitions_snippet <- paste(sapply(parsed_filtration_setups, function(x){x$definition}), collapse = "\n")
  tuples_snippet <- paste(sapply(parsed_filtration_setups, function(x){x$tuple}), collapse = ",\n  ")
  worlds_snippet <- glue::glue("const std::vector worlds { {{tuples_snippet}} };", .open = "{{", .close = "}}")
  return(paste(definitions_snippet, worlds_snippet))
}

#' @export
compile_simulation <- function(states, events, initial_conditions, filtration_setups) {
  header_snippet <- "
// [[Rcpp::depends(cfepi)]]
#include <Rcpp.h>
#include <cfepi/config.h>
#include <cfepi/sir.h>
#include <cfepi/modeling.h>
using namespace Rcpp;
// [[Rcpp::plugins(cpp23)]]
"
  states_snippet <- parse_states_to_code(states)
  events_snippet <- parse_events_to_code(events, states)
  initial_conditions_snippet <- parse_initial_conditions_to_code(initial_conditions, states)
  worlds_snippet <- parse_filtration_setups(filtration_setups)

  code_prefix <- "
// [[Rcpp::export]]
auto run_simulation(size_t epidemic_length, size_t simulation_seed) {
"

  code_postfix <- "
    const auto all_states = cfepi::run_simulation<states_t, any_event_type, any_event>(
      event_type_tuple, initial_conditions, event_rates, worlds, epidemic_length,
    simulation_seed);
    auto all_states_df = cfepi::concatenate_dataframes(all_states, \"world_index\", states);
    auto all_states_return = Rcpp::DataFrame::create(
      Named(std::get<0>(all_states_df)[0]) = std::get<0>(std::get<1>(all_states_df)),
      Named(std::get<0>(all_states_df)[1]) = std::get<1>(std::get<1>(all_states_df)),
      Named(std::get<0>(all_states_df)[2]) = std::get<2>(std::get<1>(all_states_df)),
      Named(std::get<0>(all_states_df)[3]) = std::get<3>(std::get<1>(all_states_df))
    );
    return(all_states_return);
}
"
  source_code <- glue::glue("{{header_snippet}}\n\n
{{code_prefix}}\n\n
{{states_snippet}}\n\n
{{events_snippet}}\n\n
{{initial_conditions_snippet}}\n\n
{{worlds_snippet}}\n\n
{{code_postfix}}", .open = "{{", .close = "}}")
  export_env <- new.env()
  print(source_code)
  exported <- Rcpp::sourceCpp(code = source_code, env = export_env)
  if (!("run_simulation" %in% exported$functions)) {
    stop("run simulation not properly exported.")
  }
  return(export_env$run_simulation)
}
