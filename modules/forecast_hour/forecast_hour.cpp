//standard multi sourcefile include
#include "forecast_hour.hpp"
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

//function responsible for responding with forecast for a given hour
void forecast_hour(const dpp::slashcommand_t& event){
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
    
    //get number of days from today(can't be more than 2 cause that's what free tier api supports)
    int64_t day = std::get<int64_t>(event.get_parameter(DAY));
    //get hour number
    int64_t hour = std::get<int64_t>(event.get_parameter(HOUR));

    if(hour == 24) hour = 0;
    if((day>2 || day< 0)&&(hour > 23 || hour < 0)){
        event.reply(INCORRECT_DAY_AND_HOUR);
    }else if(day>2 || day< 0){
        event.reply(INCORRECT_DAY);
    }else if(hour > 23 || hour < 0){
        event.reply(INCORRECT_HOUR);
    }

    //get city name from event object
    std::string city = std::get<std::string>(event.get_parameter(CITY));
    //try to get weather data for passed city name
	cpr::Response forecast_hour_response = cpr::Get(cpr::Url{API_URL_FORECAST},
                    cpr::Parameters{{"key", API_TOKEN}, {"q", city}, {"days", std::to_string(day+1)}});

    //handle http error codes
    switch (forecast_hour_response.status_code){
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
    nlohmann::json forecast_hour_json = nlohmann::json::parse(forecast_hour_response.text);

    //create embed with weather data and send it
    dpp::embed forecast_hour_embed = dpp::embed().
        set_color(dpp::colors::lawn_green).
        set_title(WEATHER_IN+city).
        set_url(WEATHER_SEARCH+city+ IN_DAY_URL + forecast_hour_json["forecast"]["forecastday"][day]["date"].get<std::string>() + AT_HOUR_URL + std::to_string(hour)).
        set_author(BOT_NAME, REPO, BOT_IMG).
        set_description(DATE+ forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["time"].get<std::string>()).
        add_field(
            COUNTRY_TITLE,
            forecast_hour_json["/location/country"_json_pointer].get<std::string>()
        ).
        add_field(
            TEMPERATURE_TITLE,
            to_string_with_precision<double>(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour][temp_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            FEELS_LIKE_TITLE,
            to_string_with_precision<double>(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour][feels_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            WIND_SPEED_TITLE,
            to_string_with_precision<double>(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour][wind_speed_field].get<double>()) + wind_speed_unit,
            true
        ).
        add_field(
            WIND_DIRECTION_TITLE,
            forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["wind_dir"].get<std::string>(),
            true
        ).
        add_field(
            AIR_PRESSURE_TITLE,
            to_string_with_precision<double>(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour][air_pressure_field].get<double>()) + air_pressure_unit,
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            CLOUD_COVER_TITLE,
            std::to_string(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["cloud"].get<int>()) + PERCENT,
            true
        ).
        add_field(
            UV_TITLE,
            std::to_string(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["uv"].get<int>()),
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            HUMIDITY_TITLE,
            std::to_string(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["humidity"].get<int>()) + PERCENT,
            true
        ).
        add_field(
            PRECIPITATION_TITLE,
            to_string_with_precision<double>(forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour][precipitation_field].get<double>()) + precipitation_unit,
            true
        ).
        set_image("http:" + forecast_hour_json["forecast"]["forecastday"][day]["hour"][hour]["/condition/icon"_json_pointer].get<std::string>()).
        set_timestamp(time(0));
    event.reply(dpp::message(event.command.channel_id, forecast_hour_embed));
}