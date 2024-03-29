#include <cfepi/config.h>
#include <cfepi/modeling.h>
#include <cfepi/sample_view.h>

#include <iostream>

int main() {
  const static constexpr std::string_view json_config
      = "{    \"states\": [\"S\", \"I\", \"R\", \"V\"],    \"events\":    [	{	    "
        "\"source\": [ [\"I\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": "
        "\"R\"} ],	    \"comment_rate\": \"1 / 2.25 = 0.4444444444444444\",	    "
        "\"rate\": 0.4444444444444444	},	{	    \"source\": [ [ \"I\" ], [ \"S\", "
        "\"V\" ] ],	    \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} ],	   "
        " \"comment_rate\": \"1.75 * 1 / 2.25 / 400000 = 0.000001944444\",	    \"rate\": "
        "0.000001944444	}    ],    \"initial_conditions\": {	\"S\": 399995,	\"I\": 5    },    "
        "\"worlds\": [	{	    \"event_filter\": {\"function\": \"do_nothing\"},	    "
        "\"state_modifier\": {\"function\": \"do_nothing\"},	    \"state_filter\": "
        "{\"function\": \"do_nothing\"}	},	{	    \"event_filter\": {		"
        "\"function\": \"flat_reduction_single_compartment\",		\"parameters\" : {	"
        "	    \"event_index\": 1,		    \"reduction_percentage\": 0.05,		   "
        " \"compartment_to_filter\": \"S\",		    \"index_to_filter\": 1		"
        "}	    },	    \"state_modifier\": {\"function\": \"do_nothing\"},	    "
        "\"state_filter\": {\"function\": \"do_nothing\"}	},	{	    "
        "\"event_filter\": {		\"function\": \"flat_reduction_single_compartment\",	"
        "	\"parameters\" : {		    \"event_index\": 1,		    "
        "\"reduction_percentage\": 1.0,		    \"compartment_to_filter\": \"V\",		   "
        " \"index_to_filter\": 1		}	    },	    \"state_modifier\": {	"
        "	\"function\": \"single_time_move\",		\"parameters\" : {		   "
        " \"percent_to_move\": 0.033,		    \"source_compartments\": [\"S\"],		   "
        " \"destination_compartment\": \"V\",		    \"time\": 0		}	    },	   "
        " \"state_filter\": {\"function\": \"do_nothing\"}	}    ]}";
  std::default_random_engine rand{};

  for (auto i = 0; i < 10; ++i) {
    cfepi::run_simulation_from_config<json_config>(365UL, rand());
  }
}
