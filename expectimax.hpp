#ifndef EXPECTIMAX_HPP
#define EXPECTIMAX_HPP
#include <cstdint>
#include <vector>

enum class Action {
    SHOOT_DEALER,
    SHOOT_PLAYER,
    DRINK_BEER,
    SMOKE_CIGARETTE,
    USE_MAGNIFYING_GLASS,
};

struct Node {
    bool is_dealer_turn;
    uint8_t live_round_count;
    uint8_t blank_round_count;
    uint8_t max_lives;
    uint8_t dealer_lives;
    uint8_t player_lives;
    std::vector<Action> available_dealer_item_actions;
    std::vector<Action> available_player_item_actions;
};

std::pair<Action, float> get_best_action(Node node);
bool is_terminal_node(Node node);
#endif
