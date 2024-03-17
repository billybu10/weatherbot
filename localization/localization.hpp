#pragma once
//bot message language settings (set 1 for English, 0 for German, 3 for Polish and 2 for Italian)
// i honestly doubt i'll ever add those languages, so if you want to add them/need them/are just bored you can do it yourself, just copy defines from english one and translate them  
#define BOT_LANGUAGE 1
#if BOT_LANGUAGE == 0
#include "localization_de.hpp"
#endif
#if BOT_LANGUAGE == 1
#include "localization_en.hpp"
#endif
#if BOT_LANGUAGE == 2
#include "localization_it.hpp"
#endif
#if BOT_LANGUAGE == 3
#include "localization_pl.hpp"
#endif
