#include <cstdint>
#include <vector>

#include "expectimax.hpp"

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

float eval_expectimax_performance(int sim_count, uint8_t live_round_count,
                                  uint8_t blank_round_count, uint8_t max_lives,
                                  std::vector<Action> &available_item_actions) {
	float win_count = 0;

	for (int i = 0; i < sim_count; i++) {
		Node node(false, false, false, live_round_count, blank_round_count, max_lives, max_lives,
		          max_lives, available_item_actions, available_item_actions);

		while (!node.is_terminal()) {
            if(node.is_dealer_turn) {
                for(int item_action_idx = 0; item_action_idx < node.available_dealer_item_actions.size(); item_action_idx++) {
                    // TODO: eval 
                }
            }
            else {
            }
		}

		if (node.dealer_lives == 0) {
			win_count++;
		}
	}

	return win_count / sim_count;
}
