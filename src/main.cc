#include <iostream>
#include <string>

#include "expectimax.hpp"

std::string actionToString(Action action) {
	switch (action) {
		case Action::SHOOT_DEALER:
			return "SHOOT_DEALER";
		case Action::SHOOT_PLAYER:
			return "SHOOT_PLAYER";
		case Action::DRINK_BEER:
			return "DRINK_BEER";
		case Action::SMOKE_CIGARETTE:
			return "SMOKE_CIGARETTE";
		default:
			return "UNKNOWN_ACTION";
	}
}

int main() {
	for (int dealerLives = 1; dealerLives <= 6; dealerLives++) {
		for (int playerLives = 1; playerLives <= 6; playerLives++) {
			for (int totalRounds = 2; totalRounds <= 8; totalRounds++) {
				for (int liveRounds = 0; liveRounds <= totalRounds; liveRounds++) {
					int blankRounds = totalRounds - liveRounds;

					Node node;
					node.is_dealer_turn = false;
					node.live_round_count = static_cast<uint8_t>(liveRounds);
					node.blank_round_count = static_cast<uint8_t>(blankRounds);
					node.max_lives = 6;
					node.dealer_lives = static_cast<uint8_t>(dealerLives);
					node.player_lives = static_cast<uint8_t>(playerLives);
					node.available_dealer_item_actions = {};
					node.available_player_item_actions = {Action::DRINK_BEER,
					                                      Action::SMOKE_CIGARETTE};

					auto [best_action, ev] = get_best_action(node);

					std::cout << "DealerLives=" << static_cast<int>(node.dealer_lives)
					          << ", PlayerLives=" << static_cast<int>(node.player_lives)
					          << ", LiveRounds=" << static_cast<int>(node.live_round_count)
					          << ", BlankRounds=" << static_cast<int>(node.blank_round_count)
					          << " => BestAction=" << actionToString(best_action) << ", EV=" << ev
					          << '\n';
				}
			}
		}
	}

	return 0;
}
