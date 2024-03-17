//standard multi sourcefile include
#include "forecast.hpp"
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

//function responsible for responding with forecast for whole day
void forecast(const dpp::slashcommand_t& event){
    //declare variables holding pointers to json fields
    dpp::json::json_pointer max_temp_field, min_temp_field, avg_temp_field, max_wind_speed_field, total_precipitation_field;
    //declare variables responsible for holding response unit endings
    std::string temp_unit, wind_speed_unit, precipitation_unit;
    
    //check which set of units was chosen and set variables accordingly
    if (std::holds_alternative<std::monostate>(event.get_parameter(UNITS_PARAM)))
    {
        max_temp_field = "/maxtemp_c"_json_pointer;
        min_temp_field = "/mintemp_c"_json_pointer;
        max_wind_speed_field = "/maxwind_kph"_json_pointer;
        avg_temp_field = "/avgtemp_c"_json_pointer;
        total_precipitation_field = "/totalprecip_mm"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_SI;
        wind_speed_unit = WIND_SPEED_UNIT_SI;
        precipitation_unit = PRECIPITATION_UNIT_SI;
    }else if(std::get<bool>(event.get_parameter(UNITS_PARAM))){
        max_temp_field = "/maxtemp_f"_json_pointer;
        min_temp_field = "/mintemp_f"_json_pointer;
        max_wind_speed_field = "/maxwind_mph"_json_pointer;
        avg_temp_field = "/avgtemp_f"_json_pointer;
        total_precipitation_field = "/totalprecip_in"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_IMP;
        wind_speed_unit = WIND_SPEED_UNIT_IMP;
        precipitation_unit = PRECIPITATION_UNIT_IMP;
    }else{
        max_temp_field = "/maxtemp_c"_json_pointer;
        min_temp_field = "/mintemp_c"_json_pointer;
        max_wind_speed_field = "/maxwind_kph"_json_pointer;
        avg_temp_field = "/avgtemp_c"_json_pointer;
        total_precipitation_field = "/totalprecip_mm"_json_pointer;
        temp_unit = TEMPERATURE_UNIT_SI;
        wind_speed_unit = WIND_SPEED_UNIT_SI;
        precipitation_unit = PRECIPITATION_UNIT_SI;
    }
    
    //get number of days from today(can't be more than 2 cause that's what free tier api supports)
    int64_t day = std::get<int64_t>(event.get_parameter(DAY));

    if(day>2 || day< 0){
        event.reply(INCORRECT_DAY);
    }

    //get city name from event object
    std::string city = std::get<std::string>(event.get_parameter(CITY));
    //try to get weather data for passed city name
	cpr::Response forecast_response = cpr::Get(cpr::Url{API_URL_FORECAST},
                    cpr::Parameters{{"key", API_TOKEN}, {"q", city}, {"days", std::to_string(day+1)}});

    //handle http error codes
    switch (forecast_response.status_code){
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
    nlohmann::json forecast_json = nlohmann::json::parse(forecast_response.text);

    //create embed with weather data and send it
    dpp::embed forecast_embed = dpp::embed().
        set_color(dpp::colors::lawn_green).
        set_title(WEATHER_IN+city).
        set_url(WEATHER_SEARCH+city+ IN_DAY_URL + forecast_json["forecast"]["forecastday"][day]["date"].get<std::string>()).
        set_author(BOT_NAME, REPO, BOT_IMG).
        set_description(DATE+ forecast_json["forecast"]["forecastday"][day]["date"].get<std::string>()).
        add_field(
            COUNTRY_TITLE,
            forecast_json["/location/country"_json_pointer].get<std::string>()
        ).
        add_field(
            MAX_TEMPERATURE_TITLE,
            to_string_with_precision<double>(forecast_json["forecast"]["forecastday"][day]["day"][max_temp_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            MIN_TEMPERATURE_TITLE,
            to_string_with_precision<double>(forecast_json["forecast"]["forecastday"][day]["day"][min_temp_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            AVG_TEMPERATURE_TITLE,
            to_string_with_precision<double>(forecast_json["forecast"]["forecastday"][day]["day"][avg_temp_field].get<double>()) + temp_unit,
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            MAX_WIND_SPEED_TITLE,
            to_string_with_precision<double>(forecast_json["forecast"]["forecastday"][day]["day"][max_wind_speed_field].get<double>()) + wind_speed_unit,
            true
        ).
        add_field(
            UV_TITLE,
            std::to_string(forecast_json["forecast"]["forecastday"][day]["day"]["uv"].get<int>()),
            true
        ).
        add_field(
            "",""
        ).
        add_field(
            AVG_HUMIDITY_TITLE,
            std::to_string(forecast_json["forecast"]["forecastday"][day]["day"]["avghumidity"].get<int>()) + PERCENT,
            true
        ).
        add_field(
            TOTAL_PRECIPITATION_TITLE,
            to_string_with_precision<double>(forecast_json["forecast"]["forecastday"][day]["day"][total_precipitation_field].get<double>()) + precipitation_unit,
            true
        ).
        set_image("http:" + forecast_json["forecast"]["forecastday"][day]["day"]["/condition/icon"_json_pointer].get<std::string>()).
        set_timestamp(time(0));
    event.reply(dpp::message(event.command.channel_id, forecast_embed));
}