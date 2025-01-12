#ifndef TEST_HPP
#define TEST_HPP
#include <cstdint>
#include <vector>

#include "expectimax.hpp"

float eval_expectimax_performance(int sim_count, uint8_t live_round_count,
                                  uint8_t blank_round_count, uint8_t max_lives,
                                  std::vector<Action> available_item_actions);
#endif
