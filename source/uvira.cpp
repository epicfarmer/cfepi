#include <cfepi/config.h>
#include <cfepi/modeling.h>
#include <cfepi/sample_view.h>

#include <iostream>

int main() {
  const static constexpr std::string_view json_config
      = "{    \"states\": [\"S\", \"I\", \"R\", \"V\"],    \"events\":    [	{	    "
        "\"source\": [ [\"I\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": "
        "\"R\"} ],	    \"rate\": 0.444444	},	{	    \"source\": [ [ \"I\" ], [ "
        "\"S\", \"V\" ] ],	    \"destination\": [ {\"index\": 1, \"compartment\": \"I\"} "
        "],	    \"comment_rate\": \"1.05 * .44444444444444 / 273613 = 0.000001705572\",	   "
        " \"rate\": 0.000001705572	}    ],    \"initial_conditions\": {	\"S\": 273612,	"
        "\"I\": 1    },    \"worlds\": [	{	    \"event_filter\": {		"
        "\"function\": \"flat_reduction_single_compartment\",		\"parameters\" : {	"
        "	    \"event_index\": 1,		    \"reduction_percentage\": 0.1,		   "
        " \"compartment_to_filter\": \"V\"		    \"index_to_filter\": 1		"
        "}	    },	    \"state_modifier\": {		\"function\": "
        "\"single_time_move\",		\"parameters\" : {		    \"percent_to_move\": "
        "0.24,		    \"source_compartments\": [\"S\"],		    "
        "\"destination_compartment\": \"V\",		    \"time\": 0		}	    },	   "
        " \"state_filter\": {		\"function\": \"strict_incidence_filter\",		"
        "\"parameters\" : {		    \"compartment\" : \"I\",		    \"counts\": "
        "[			1,    1,    1,    2,    1,    1,    1,    1,    1,    1,    1,    "
        "1,    1,    1,    2,    2,    6,    1,    1,    3,    3,    4,    8,    2,    4,    2,    "
        "7,    3,    9,    4,    5,    7,    8,    4,    7,    7,    6,    7,    9,    7,    9,    "
        "12,    7,    13,    15,    8,    21,    17,    11,    11,    14,    11,    3,    4,    "
        "14,    9,    7,    8 ]		}	    }	}    ]}";
  std::default_random_engine rand{};

  for (auto i = 0; i < 100; ++i) {
    // cfepi::run_simulation_from_config<json_config>(10UL ,rand());
    cfepi::run_simulation_from_config<json_config>(365UL, rand());
  }
}
