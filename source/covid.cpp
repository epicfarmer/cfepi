#include <iostream>

#include <cfepi/modeling.h>
#include <cfepi/config.h>
#include <cfepi/sample_view.h>

int main() {

  const static constexpr std::string_view json_config = "{    \"states\": [\"S\", \"E\", \"I1\", \"I2\", \"I3\", \"R\"],    \"events\":    [	{	    \"source\": [ [\"E\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"I1\"} ],	    \"rate\": 0.192307692307692	},	{	    \"source\": [ [\"I1\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"I2\"} ],	    \"rate\": 0.75	},	{	    \"source\": [ [\"I2\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"I3\"} ],	    \"rate\": 0.75	},	{	    \"source\": [ [\"I3\"] ],	    \"destination\": [ {\"index\": 0, \"compartment\": \"R\"} ],	    \"rate\": 0.75	},	{	    \"source\": [ [ \"I1\", \"I2\", \"I3\"], [ \"S\" ] ],	    \"destination\": [ {\"index\": 1, \"compartment\": \"E\"} ],	    \"comment_rate\": \"2.3 * 0.25 / 33100265 = 0.00000001737146\",	    \"rate\": 0.00000001737146	}    ],    \"initial_conditions\": {	\"S\": 33100265,	\"I1\": 1    },    \"worlds\": [	{    \"event_filter\": {\"function\": \"do_nothing\"},    \"state_filter\": {\"function\": \"do_nothing\"},    \"state_modifier\": {\"function\": \"do_nothing\"}	}    ]}";
  for (auto i __attribute__((unused)) : std::ranges::views::iota(0,5)) {
    std::cout << "New run\n";
    cfepi::run_simulation_from_config<json_config>(365*3, 1000);
  }

}
