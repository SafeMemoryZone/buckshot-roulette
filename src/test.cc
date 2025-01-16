#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

#include "expectimax.hpp"

std::random_device dev;
std::mt19937 rng(dev());

void Node::apply_shoot_dealer_live(void) {
	this->dealer_lives--;
	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = !this->is_dealer_turn;
}

void Node::apply_shoot_dealer_blank(void) {
	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = true;
}

void Node::apply_shoot_player_live(void) {
	this->player_lives--;
	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = !this->is_dealer_turn;
}

void Node::apply_shoot_player_blank(void) {
	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = false;
}

int Node::find_item_action_idx(Action item_action) const {
	if (this->is_dealer_turn) {
		for (int item_action_idx = 0; item_action_idx < this->available_dealer_item_actions.size();
		     item_action_idx++) {
			if (this->available_dealer_item_actions.at(item_action_idx) == item_action) {
				return item_action_idx;
			}
		}
	}
	else {
		for (int item_action_idx = 0; item_action_idx < this->available_player_item_actions.size();
		     item_action_idx++) {
			if (this->available_player_item_actions.at(item_action_idx) == item_action) {
				return item_action_idx;
			}
		}
	}

	assert(false && "no action found");
	return -1;
}

void Node::apply_drink_beer_live(void) {
	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;

	int item_action_idx = this->find_item_action_idx(Action::DRINK_BEER);

	if (this->is_dealer_turn) {
		this->remove_dealer_item_action(item_action_idx);
	}
	else {
		this->remove_player_item_action(item_action_idx);
	}
}

void Node::apply_drink_beer_blank(void) {
	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;

	int item_action_idx = this->find_item_action_idx(Action::DRINK_BEER);

	if (this->is_dealer_turn) {
		this->remove_dealer_item_action(item_action_idx);
	}
	else {
		this->remove_player_item_action(item_action_idx);
	}
}

void Node::apply_smoke_cigarette(void) {
	if (this->is_dealer_turn) {
		this->dealer_lives++;
	}
	else {
		this->player_lives++;
	}

	int item_action_idx = this->find_item_action_idx(Action::SMOKE_CIGARETTE);

	if (this->is_dealer_turn) {
		this->remove_dealer_item_action(item_action_idx);
	}
	else {
		this->remove_player_item_action(item_action_idx);
	}
}

void Node::apply_magnify_live(void) {
	this->curr_is_live = true;
	int item_action_idx = this->find_item_action_idx(Action::USE_MAGNIFYING_GLASS);

	if (this->is_dealer_turn) {
		this->remove_dealer_item_action(item_action_idx);
	}
	else {
		this->remove_player_item_action(item_action_idx);
	}
}

void Node::apply_magnify_blank(void) {
	this->curr_is_blank = true;

	int item_action_idx = this->find_item_action_idx(Action::USE_MAGNIFYING_GLASS);

	if (this->is_dealer_turn) {
		this->remove_dealer_item_action(item_action_idx);
	}
	else {
		this->remove_player_item_action(item_action_idx);
	}
}

float eval_expectimax_performance(int sim_count, uint8_t live_round_count,
                                  uint8_t blank_round_count, uint8_t max_lives,
                                  std::vector<Action> &available_item_actions) {
	float win_count = 0;

	for (int i = 0; i < sim_count; i++) {
		Node node(false, false, false, live_round_count, blank_round_count, max_lives, max_lives,
		          max_lives, available_item_actions, available_item_actions);

		while (!node.is_terminal()) {
			std::uniform_int_distribution<std::mt19937::result_type> round_dist(
			    1, node.live_round_count + node.blank_round_count);

			if (node.is_dealer_turn) {
				std::uniform_int_distribution<std::mt19937::result_type> dealer_choice_dist(1, 2);

				std::vector<Action> chosen_item_actions;

				for (const Action item_action : node.available_dealer_item_actions) {
					switch (item_action) {
						case Action::DRINK_BEER:
							if (node.curr_is_live || node.is_last_round()) break;
							chosen_item_actions.emplace_back(item_action);
							break;
						case Action::SMOKE_CIGARETTE:
							if (node.dealer_lives == node.max_lives) break;
							chosen_item_actions.emplace_back(item_action);
							break;
						case Action::USE_MAGNIFYING_GLASS:
							if (node.curr_is_live || node.curr_is_blank || node.is_last_round())
								break;
							chosen_item_actions.emplace_back(item_action);
							break;
						default:
							assert(false && "invalid item action");
					}
				}

				for (const Action item_action : chosen_item_actions) {
					switch (item_action) {
						case Action::DRINK_BEER:
							if (node.curr_is_live || node.is_last_round()) break;
							if (round_dist(rng) <= node.live_round_count) {
								node.apply_drink_beer_live();
							}
							else {
								node.apply_drink_beer_blank();
							}
							break;
						case Action::SMOKE_CIGARETTE:
							if (node.dealer_lives == node.max_lives) break;
							node.apply_smoke_cigarette();
							break;
						case Action::USE_MAGNIFYING_GLASS:
							if (node.curr_is_live || node.curr_is_blank || node.is_last_round())
								break;
							if (round_dist(rng) <= node.live_round_count) {
								node.apply_magnify_live();
							}
							else {
								node.apply_magnify_blank();
							}
							break;
						default:
							assert(false && "invalid item action");
					}
				}

				if ((node.live_round_count + node.blank_round_count) == 1) {
					if (node.live_round_count == 1) {
						node.apply_shoot_player_live();
					}
					node.apply_shoot_dealer_blank();
				}

				else {
					if (dealer_choice_dist(rng) == 1) {
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_shoot_dealer_live();
						}
						else {
							node.apply_shoot_dealer_blank();
						}
					}
					else {
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_shoot_player_live();
						}
						else {
							node.apply_shoot_player_blank();
						}
					}
				}
			}
			else {
				auto [best_action, ev] = node.get_best_action();

				switch (best_action) {
					case Action::SHOOT_DEALER:
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_shoot_dealer_live();
						}
						else {
							node.apply_shoot_dealer_blank();
						}
						break;
					case Action::SHOOT_PLAYER:
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_shoot_player_live();
						}
						else {
							node.apply_shoot_player_blank();
						}
						break;
					case Action::DRINK_BEER:
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_drink_beer_live();
						}
						else {
							node.apply_drink_beer_blank();
						}
						break;
					case Action::SMOKE_CIGARETTE:
						node.apply_smoke_cigarette();
						break;
					case Action::USE_MAGNIFYING_GLASS:
						if (round_dist(rng) <= node.live_round_count) {
							node.apply_magnify_live();
						}
						else {
							node.apply_magnify_blank();
						}
						break;
				}
			}

			// reset round
			if ((node.live_round_count + node.blank_round_count) == 0) {
				node.live_round_count = live_round_count;
				node.blank_round_count = blank_round_count;
				node.available_dealer_item_actions = available_item_actions;
				node.available_player_item_actions = available_item_actions;
			}
		}

		if (node.dealer_lives == 0) {
			win_count++;
		}
	}

	return win_count / sim_count;
}
