//standard multi sourcefile include
#include "weather.hpp"
//c++ standard libs
#include <string>
//d++(https://dpp.dev) installed into os
#include <dpp/dpp.h>
#include <dpp/queues.h>
//C++ Requests (https://docs.libcpr.org/) include
#include <cpr/cpr.h>
//Nlohmann's json library (https://json.nlohmann.me/)
#include <nlohmann/json.hpp>
//weatherapi and discord token macros
#include "../../include/tokens.hpp"
//localization
#include "../../localization/localization.hpp"
//links and language-indepenent strings
#include "../../include/strings_and_links.hpp"
//custom tostring
#include "../../include/tostring/tostring.hpp"


//function responsible for responding with current weather
void weather(const dpp::slashcommand_t& event){
    //declare variables holding pointers to json fields
    dpp::json::json_pointer temp_field, feels_field, wind_speed_field, air_pressure_field, precipitation_field;
    //declare variables responsible for holding response unit endings
    std::string temp_unit, wind_speed_unit, air_pressure_unit, precipitation_unit;

    //check which set of units was chosen and set variables accordingly
    if (std::holds_alternative<std::monostate>(event.get_parameter(UNITS_PARAM)))
    {
        temp_field = "/temp_c"_json_pointer;
        feels_field = "/feelslike_c"_json_pointer;
        wind_speed_field = "/wind_kph"_json_pointer;
        air_pressure_field = "/pressure_mb"_json_pointer;
        precipitation_field = "/precip_mm"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_SI;
        wind_speed_unit = WIND_SPEED_UNIT_SI;
        air_pressure_unit = AIR_PRESSURE_UNIT_SI;
        precipitation_unit = PRECIPITATION_UNIT_SI;
    }else if(std::get<bool>(event.get_parameter(UNITS_PARAM))){
        temp_field = "/temp_f"_json_pointer;
        feels_field = "/feelslike_f"_json_pointer;
        wind_speed_field = "/wind_mph"_json_pointer;
        air_pressure_field = "/pressure_in"_json_pointer;
        precipitation_field = "/precip_in"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_IMP;
        wind_speed_unit = WIND_SPEED_UNIT_IMP;
        air_pressure_unit = AIR_PRESSURE_UNIT_IMP;
        precipitation_unit = PRECIPITATION_UNIT_IMP;
    }else{
        temp_field = "/temp_c"_json_pointer;
        feels_field = "/feelslike_c"_json_pointer;
        wind_speed_field = "/wind_kph"_json_pointer;
        air_pressure_field = "/pressure_mb"_json_pointer;
        precipitation_field = "/precip_mm"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_SI;
        wind_speed_unit = WIND_SPEED_UNIT_SI;
        air_pressure_unit = AIR_PRESSURE_UNIT_SI;
        precipitation_unit = PRECIPITATION_UNIT_SI;
    }

    //get city name from event object
    std::string city = std::get<std::string>(event.get_parameter(CITY));
    //try to get weather data for passed city name
	cpr::Response weather_response = cpr::Get(cpr::Url{API_URL_CURRENT},
                    cpr::Parameters{{"key", API_TOKEN}, {"q", city}});

    //handle http error codes
    switch (weather_response.status_code){
        case 200:
            break;
        case 403:
            event.reply(ERROR_403);
            return;
            break;
        case 400:
            event.reply(ERROR_400);
            return;
            break;
        case 429:
            event.reply(ERROR_429);
            return;
            break;
        default:
            event.reply(UNKNOWN_HTTP_ERROR);
            return;
            break;
    }

    //parse http response to json library
    nlohmann::json weather_json = nlohmann::json::parse(weather_response.text);

    //create embed with weather data and send it
    dpp::embed weather_embed = dpp::embed().
        set_color(dpp::colors::lawn_green).
        set_title(WEATHER_IN+city).
        set_url(WEATHER_SEARCH+city).
        set_author(BOT_NAME, REPO, BOT_IMG).
        set_description(DATA_COLLECTED_AT_DESC + weather_json["/current/last_updated"_json_pointer].get<std::string>()).
        add_field(
            COUNTRY_TITLE,
            weather_json["/location/country"_json_pointer].get<std::string>()
        ).
        add_field(
            TEMPERATURE_TITLE,
            to_string_with_precision<double>(weather_json["current"][temp_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            FEELS_LIKE_TITLE,
            to_string_with_precision<double>(weather_json["current"][feels_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            WIND_SPEED_TITLE,
            to_string_with_precision<double>(weather_json["current"][wind_speed_field].get<double>()) + wind_speed_unit,
            true
        ).
        add_field(
            WIND_DIRECTION_TITLE,
            weather_json["/current/wind_dir"_json_pointer].get<std::string>(),
            true
        ).
        add_field(
            AIR_PRESSURE_TITLE,
            to_string_with_precision<double>(weather_json["current"][air_pressure_field].get<double>()) + air_pressure_unit,
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            CLOUD_COVER_TITLE,
            std::to_string(weather_json["/current/cloud"_json_pointer].get<int>()) + PERCENT,
            true
        ).
        add_field(
            UV_TITLE,
            std::to_string(weather_json["/current/uv"_json_pointer].get<int>()),
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            HUMIDITY_TITLE,
            std::to_string(weather_json["/current/humidity"_json_pointer].get<int>()) + PERCENT,
            true
        ).
        add_field(
            PRECIPITATION_TITLE,
            to_string_with_precision<double>(weather_json["current"][precipitation_field].get<double>()) + precipitation_unit,
            true
        ).
        set_image("http:" + weather_json["/current/condition/icon"_json_pointer].get<std::string>()).
        set_timestamp(time(0));
    event.reply(dpp::message(event.command.channel_id, weather_embed));
}