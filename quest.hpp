#pragma once

#define QUOTE(TEXT) #TEXT
#define TO_STRING(TEXT) QUOTE(TEXT)

#if !(0 < QUEST && QUEST <= QUESTS_NB)
#error "Quest index is invalid"
#endif

constexpr static uint8_t quest_index = QUEST;
constexpr static uint8_t quests_nb = QUESTS_NB;
