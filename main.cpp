//Powered by WeatherAPI.com(https://www.weatherapi.com/)

//c++ standard libs
#include <string>
//d++(https://dpp.dev)
#include <dpp/dpp.h>
#include <dpp/queues.h>
//uberswitch(https://github.com/falemagn/uberswitch)
#define UBERSWITCH_ALLOW_NESTING 1
#include <uberswitch/uberswitch.hpp>
//weatherapi and discord token macros
#include "include/tokens.hpp"
//modules handling individual commands
#include "modules/weather/weather.hpp" 
#include "modules/forecast/forecast.hpp" 
#include "modules/air_pollution/air_pollution.hpp"
#include "modules/forecast_hour/forecast_hour.hpp"
//localization
#include "localization/localization.hpp"
//links and language-indepenent strings
#include "include/strings_and_links.hpp"

int main() {
	//create new cluster
	dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());

	//check which command is chosen and call coresponding function
	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
		uswitch (event.command.get_command_name()) {
			ucase (WEATHER_COMMAND):
				weather(event);
				break;

			ucase (FORECAST_SUMMARY_COMMAND):
				forecast(event);
				break;

			ucase (POLLUTION_COMMAND):
				air_pollution(event);
				break;

			ucase (FORECAST_HOUR_COMMAND):
				forecast_hour(event);
				break;

			default:
				event.reply(UNKNOWN_COMMAND);
				break;
		}
	});

	//register all commands with their options when cluster is ready
	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand(WEATHER_COMMAND, WEATHER_DESC, bot.me.id)
				.add_option(
					dpp::command_option(dpp::co_string, CITY, CITY_WEATHER_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_boolean, UNITS_PARAM, UNITS_PARAM_DESC, false)
				)
			);
			bot.global_command_create(dpp::slashcommand(FORECAST_SUMMARY_COMMAND, FORECAST_SUMMARY_DESC, bot.me.id)
				.add_option(
					dpp::command_option(dpp::co_string, CITY, CITY_FORECAST_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_integer, DAY, DAY_FORECAST_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_boolean, UNITS_PARAM, UNITS_PARAM_DESC, false)
				)
			);
			bot.global_command_create(dpp::slashcommand(FORECAST_HOUR_COMMAND, FORECAST_HOUR_DESC, bot.me.id)
				.add_option(
					dpp::command_option(dpp::co_string, CITY, CITY_FORECAST_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_integer, DAY, DAY_FORECAST_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_integer, HOUR, HOUR_FORECAST_DESC, true)
				)
				.add_option(
					dpp::command_option(dpp::co_boolean, UNITS_PARAM, UNITS_PARAM_DESC, false)
				)
			);
			bot.global_command_create(dpp::slashcommand(POLLUTION_COMMAND, POLLUTION_DESC, bot.me.id)
				.add_option(
					dpp::command_option(dpp::co_string, CITY, CITY_POLLUTION_DESC, true)
				)
			);
		}
	});

	//send a greet message when bot is added to a guild
	bot.on_guild_create([&bot](const dpp::guild_create_t& event){
		dpp::embed hello_embed = dpp::embed()
		.set_color(dpp::colors::lawn_green)
		.set_title(HELLO_TITLE)
		.set_author(BOT_NAME, REPO, BOT_IMG)
		.set_description(HELLO_FIELD)
		.set_url(REPO_README);
		bot.message_create(dpp::message(event.created->system_channel_id, hello_embed));
	});

	bot.start(dpp::st_wait);
}