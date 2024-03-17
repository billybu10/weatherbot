//standard multi sourcefile include
#include "air_pollution.hpp"
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

//function responsible for responding with current air pollution
void air_pollution(const dpp::slashcommand_t& event){
    //get city name from event object
    std::string city = std::get<std::string>(event.get_parameter(CITY));
    //try to get air quality data for passed city name
	cpr::Response air_quality_response = cpr::Get(cpr::Url{API_URL_CURRENT},
                    cpr::Parameters{{"key", API_TOKEN}, {"q", city}, {"aqi", "yes"}});
    
    //handle http error codes
    switch (air_quality_response.status_code){
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
    nlohmann::json air_quality_json = nlohmann::json::parse(air_quality_response.text);
    //change embed accent color based on british air quality index (i'd prefer an eu one, but this api comes only with gb and us ones, and i took british because i think they have a more restrictive brackets)
    uint32_t air_quality_embed_color;
    switch(air_quality_json["/current/air_quality/gb-defra-index"_json_pointer].get<int>()){
        case 1: case 2: case 3:
            air_quality_embed_color = dpp::colors::neon_green;
            break;
        case 4: case 5: case 6:
            air_quality_embed_color = dpp::colors::banana_yellow;
            break;
        case 7: case 8: case 9:
            air_quality_embed_color = dpp::colors::red;
            break;
        case 10:
            air_quality_embed_color = dpp::colors::fuchsia;
            break;
        default :
            air_quality_embed_color = dpp::colors::light_gray;
            break;
    };

    //create embed with air quality data and send it
    dpp::embed air_quality_embed = dpp::embed().
        set_color(air_quality_embed_color).
        set_title(AIR_QUALITY_IN+city).
        set_url(AIR_QUALITY_SEARCH+city).
        set_author(BOT_NAME, REPO, BOT_IMG).
        set_description(DATA_COLLECTED_AT_DESC + air_quality_json["/current/last_updated"_json_pointer].get<std::string>()).
        add_field(
            COUNTRY_TITLE,
            air_quality_json["/location/country"_json_pointer].get<std::string>()
        ).
        add_field(
            CO_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/co"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            NO2_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/no2"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            O3_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/o3"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            SO2_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/so2"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            PM2_5_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/pm2_5"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            PM10_TITLE,
            to_string_with_precision<double>(air_quality_json["/current/air_quality/pm10"_json_pointer].get<double>()) + AIR_QUALITY_UNIT
        ).
        add_field(
            UK_DEFRA_TITLE,
            std::to_string(air_quality_json["/current/air_quality/gb-defra-index"_json_pointer].get<int>())
        ).
        set_timestamp(time(0));
    event.reply(dpp::message(event.command.channel_id, air_quality_embed));
}