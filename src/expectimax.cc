#include "expectimax.hpp"

#include <array>
#include <cassert>
#include <limits>

Node::Node(bool is_dealer_turn, bool curr_is_live, bool curr_is_blank, uint8_t live_round_count,
           uint8_t blank_round_count, uint8_t max_lives, uint8_t dealer_lives, uint8_t player_lives,
           const std::vector<Action> &available_dealer_item_actions,
           const std::vector<Action> &available_player_item_actions) {
	this->is_dealer_turn = is_dealer_turn;
	this->curr_is_live = curr_is_live;
	this->curr_is_blank = curr_is_blank;
	this->live_round_count = live_round_count;
	this->blank_round_count = blank_round_count;
	this->max_lives = max_lives;
	this->dealer_lives = dealer_lives;
	this->player_lives = player_lives;
	this->available_dealer_item_actions = available_dealer_item_actions;
	this->available_player_item_actions = available_player_item_actions;
}

bool Node::is_only_live_rounds(void) const {
	return this->live_round_count > 0 && this->blank_round_count == 0;
}

bool Node::is_only_blank_rounds(void) const {
	return this->blank_round_count > 0 && this->live_round_count == 0;
}

bool Node::is_last_round(void) const {
	return (this->live_round_count + this->blank_round_count) == 1;
}

bool Node::is_terminal(void) const {
	return this->dealer_lives == 0 || this->player_lives == 0 ||
	       (this->live_round_count + this->blank_round_count) == 0;
}

void Node::remove_dealer_item_action(int item_action_idx) {
	this->available_dealer_item_actions.at(item_action_idx) =
	    this->available_dealer_item_actions.back();
	this->available_dealer_item_actions.pop_back();
}

void Node::remove_player_item_action(int item_action_idx) {
	this->available_player_item_actions.at(item_action_idx) =
	    this->available_player_item_actions.back();
	this->available_player_item_actions.pop_back();
}

float Node::eval(void) const {
	// TODO: Improve eval

	if (this->player_lives == 0) {
		return -100;
	}

	if (this->dealer_lives == 0) {
		return 100;
	}

	return (this->player_lives - this->dealer_lives) * 10;
}

std::array<Node, 4> Node::get_states_after_shoot(void) const {
	Node shoot_dealer_blank(true, false, false, this->live_round_count,
	                        static_cast<uint8_t>(this->blank_round_count - 1), this->max_lives,
	                        this->dealer_lives, this->player_lives,
	                        this->available_dealer_item_actions,
	                        this->available_player_item_actions);
	Node shoot_dealer_live(
	    !this->is_dealer_turn, false, false, static_cast<uint8_t>(this->live_round_count - 1),
	    this->blank_round_count, this->max_lives, static_cast<uint8_t>(this->dealer_lives - 1),
	    this->player_lives, this->available_dealer_item_actions,
	    this->available_player_item_actions);
	Node shoot_player_blank(false, false, false, this->live_round_count,
	                        static_cast<uint8_t>(this->blank_round_count - 1), this->max_lives,
	                        this->dealer_lives, this->player_lives,
	                        this->available_dealer_item_actions,
	                        this->available_player_item_actions);
	Node shoot_player_live(
	    !this->is_dealer_turn, false, false, static_cast<uint8_t>(this->live_round_count - 1),
	    this->blank_round_count, this->max_lives, this->dealer_lives,
	    static_cast<uint8_t>(this->player_lives - 1), this->available_dealer_item_actions,
	    this->available_player_item_actions);

	return {shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live};
}

float Node::expectimax(void) const {
	if (this->is_terminal()) {
		return this->eval();
	}

	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    this->get_states_after_shoot();
	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;

	if (this->is_dealer_turn) {
		const float item_pickup_probability = 1.0f / this->available_dealer_item_actions.size();
		/*
		 * The dealer AI acts as follows:
		 * - It always knows the last round type and acts accordingly
		 * - If it doesn't know the currect round type, it flips a coin
		 * - Before shooting, it iterates through his items in the order they spawned (we assume the
		 * order is random) and decides if he wants to use them.
		 * Item usages:
		 * - Beer: If its not the last round and he the known round (if known) isn't live
		 * - Cigarettes: If the dealer's health is not full
		 * - Magnifying Glass: If he doesn't already know the current round and it isn't the last
		 * one
		 */

		bool chose_action = false;
		float ev_after_item_usage = 0.0f;

		for (int item_action_idx = 0; item_action_idx < this->available_dealer_item_actions.size();
		     item_action_idx++) {
			const Action item_action = this->available_dealer_item_actions.at(item_action_idx);

			switch (item_action) {
				case Action::DRINK_BEER:
					if (this->curr_is_live || this->is_last_round()) break;
					ev_after_item_usage +=
					    this->drink_beer(item_pickup_probability, item_action_idx);
					chose_action = true;
					break;
				case Action::SMOKE_CIGARETTE:
					if (this->dealer_lives == this->max_lives) break;
					ev_after_item_usage +=
					    this->smoke_cigarette(item_pickup_probability, item_action_idx);
					chose_action = true;
					break;
				case Action::USE_MAGNIFYING_GLASS:
					if (this->curr_is_live || this->curr_is_blank || this->is_last_round()) break;
					ev_after_item_usage +=
					    this->use_magnifying_glass(item_pickup_probability, item_action_idx);
					chose_action = true;
					break;
				default:
					assert(false && "invalid item action");
			}
		}

		if (chose_action) {
			return ev_after_item_usage;
		}

		if (this->is_last_round()) {
			if (this->live_round_count == 1) {
				return shoot_player_live.eval();
			}

			return shoot_dealer_blank.eval();
		}

		if (this->is_only_live_rounds()) {
			return shoot_dealer_live.expectimax() * 0.5f + shoot_player_live.expectimax() * 0.5f;
		}

		if (this->is_only_blank_rounds()) {
			return shoot_dealer_blank.expectimax() * 0.5f + shoot_player_blank.expectimax() * 0.5f;
		}

		return shoot_dealer_live.expectimax() * probability_live * 0.5f +
		       shoot_dealer_blank.expectimax() * probability_blank * 0.5f +
		       shoot_player_live.expectimax() * probability_live * 0.5f +
		       shoot_player_blank.expectimax() * probability_blank * 0.5f;
	}

	float best_ev = std::numeric_limits<float>::lowest();

	for (int item_action_idx = 0; item_action_idx < this->available_player_item_actions.size();
	     item_action_idx++) {
		const Action item_action = this->available_player_item_actions.at(item_action_idx);
		switch (item_action) {
			case Action::DRINK_BEER:
				if (this->curr_is_live || this->curr_is_blank || this->is_only_blank_rounds())
					break;
				best_ev = std::max(this->drink_beer(1.0f, item_action_idx), best_ev);
				break;
			case Action::SMOKE_CIGARETTE:
				if (this->player_lives == this->max_lives) break;
				best_ev = std::max(this->smoke_cigarette(1.0f, item_action_idx), best_ev);
				break;
			case Action::USE_MAGNIFYING_GLASS:
				if (this->curr_is_live || this->curr_is_blank || this->is_only_live_rounds() ||
				    this->is_only_blank_rounds())
					break;
				best_ev = std::max(this->use_magnifying_glass(1.0f, item_action_idx), best_ev);
				break;
			default:
				assert(false && "invalid item action");
		}
	}

	if (this->is_only_live_rounds()) {
		best_ev = std::max(shoot_dealer_live.expectimax(), best_ev);
	}
	else if (this->is_only_blank_rounds()) {
		best_ev = std::max(this->eval(), best_ev);
	}
	else {
		best_ev = std::max(shoot_dealer_live.expectimax() * probability_live +
		                       shoot_dealer_blank.expectimax() * probability_blank,
		                   best_ev);
		best_ev = std::max(shoot_player_live.expectimax() * probability_live +
		                       shoot_player_blank.expectimax() * probability_blank,
		                   best_ev);
	}

	return best_ev;
}

float Node::drink_beer(float item_pickup_probability, int item_action_idx) const {
	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;

	if (this->is_only_live_rounds()) {
		Node eject_live(this->is_dealer_turn, false, false,
		                static_cast<uint8_t>(this->live_round_count - 1), this->blank_round_count,
		                this->max_lives, this->dealer_lives, this->player_lives,
		                this->available_dealer_item_actions, this->available_player_item_actions);

		if (this->is_dealer_turn) {
			eject_live.remove_dealer_item_action(item_action_idx);
		}
		else {
			eject_live.remove_player_item_action(item_action_idx);
		}

		return eject_live.expectimax() * item_pickup_probability;
	}
	if (this->is_only_blank_rounds()) {
		Node eject_blank(this->is_dealer_turn, false, false, this->live_round_count,
		                 static_cast<uint8_t>(this->blank_round_count - 1), this->max_lives,
		                 this->dealer_lives, this->player_lives,
		                 this->available_dealer_item_actions, this->available_player_item_actions);

		if (this->is_dealer_turn) {
			eject_blank.remove_dealer_item_action(item_action_idx);
		}
		else {
			eject_blank.remove_player_item_action(item_action_idx);
		}

		return eject_blank.expectimax() * item_pickup_probability;
	}

	Node eject_live(this->is_dealer_turn, false, false,
	                static_cast<uint8_t>(this->live_round_count - 1), this->blank_round_count,
	                this->max_lives, this->dealer_lives, this->player_lives,
	                this->available_dealer_item_actions, this->available_player_item_actions);

	if (this->is_dealer_turn) {
		eject_live.remove_dealer_item_action(item_action_idx);
	}
	else {
		eject_live.remove_player_item_action(item_action_idx);
	}

	Node eject_blank(this->is_dealer_turn, false, false, this->live_round_count,
	                 static_cast<uint8_t>(this->blank_round_count - 1), this->max_lives,
	                 this->dealer_lives, this->player_lives, this->available_dealer_item_actions,
	                 this->available_player_item_actions);

	if (this->is_dealer_turn) {
		eject_blank.remove_dealer_item_action(item_action_idx);
	}
	else {
		eject_blank.remove_player_item_action(item_action_idx);
	}

	return eject_live.expectimax() * item_pickup_probability * probability_live +
	       eject_blank.expectimax() * item_pickup_probability * probability_blank;
}

float Node::smoke_cigarette(float item_pickup_probability, int item_action_idx) const {
	Node smoked(
	    this->is_dealer_turn, false, false, this->live_round_count, this->blank_round_count,
	    this->max_lives,
	    this->is_dealer_turn ? static_cast<uint8_t>(this->dealer_lives + 1) : this->dealer_lives,
	    !this->is_dealer_turn ? static_cast<uint8_t>(this->player_lives + 1) : this->player_lives,
	    this->available_dealer_item_actions, this->available_player_item_actions);

	if (this->is_dealer_turn) {
		smoked.remove_dealer_item_action(item_action_idx);
	}
	else {
		smoked.remove_player_item_action(item_action_idx);
	}

	return smoked.expectimax() * item_pickup_probability;
}

float Node::use_magnifying_glass(float item_pickup_probability, int item_action_idx) const {
	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;

	Node see_live(this->is_dealer_turn, true, false, this->live_round_count,
	              this->blank_round_count, this->max_lives, this->dealer_lives, this->player_lives,
	              this->available_dealer_item_actions, this->available_player_item_actions);

	if (this->is_dealer_turn) {
		see_live.remove_dealer_item_action(item_action_idx);
	}
	else {
		see_live.remove_player_item_action(item_action_idx);
	}

	Node see_blank(this->is_dealer_turn, false, true, this->live_round_count,
	               this->blank_round_count, this->max_lives, this->dealer_lives, this->player_lives,
	               this->available_dealer_item_actions, this->available_player_item_actions);

	if (this->is_dealer_turn) {
		see_blank.remove_dealer_item_action(item_action_idx);
	}
	else {
		see_blank.remove_player_item_action(item_action_idx);
	}

	return see_live.expectimax() * probability_live * item_pickup_probability +
	       see_blank.expectimax() * probability_blank * item_pickup_probability;
}

std::pair<Action, float> Node::get_best_action(void) const {
	float best_item_ev = std::numeric_limits<float>::lowest();
	Action best_item_action = Action::SHOOT_DEALER;

	float shoot_player_ev = std::numeric_limits<float>::lowest();
	float shoot_dealer_ev = std::numeric_limits<float>::lowest();

	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;
	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    this->get_states_after_shoot();

	for (int item_action_idx = 0; item_action_idx < this->available_player_item_actions.size();
	     item_action_idx++) {
		const Action item_action = this->available_player_item_actions.at(item_action_idx);

		switch (item_action) {
			case Action::DRINK_BEER: {
				if (this->curr_is_blank || this->is_only_blank_rounds()) break;
				const float ev = this->drink_beer(1.0f, item_action_idx);
				if (ev > best_item_ev) {
					best_item_ev = ev;
					best_item_action = Action::DRINK_BEER;
				}
				break;
			}
			case Action::SMOKE_CIGARETTE: {
				if (this->player_lives == this->max_lives) break;
				const float ev = this->smoke_cigarette(1.0f, item_action_idx);
				if (ev > best_item_ev) {
					best_item_ev = ev;
					best_item_action = Action::SMOKE_CIGARETTE;
				}
				break;
			}
			case Action::USE_MAGNIFYING_GLASS: {
				if (this->curr_is_live || this->curr_is_blank || this->is_only_live_rounds() ||
				    this->is_only_blank_rounds())
					break;
				const float ev = this->use_magnifying_glass(1.0f, item_action_idx);
				if (ev > best_item_ev) {
					best_item_ev = ev;
					best_item_action = Action::USE_MAGNIFYING_GLASS;
				}
				break;
			}
			default:
				assert(false && "invalid item action");
		}
	}

	if (this->is_only_live_rounds()) {
		shoot_dealer_ev = shoot_dealer_live.expectimax();
	}
	else if (this->is_only_blank_rounds()) {
		shoot_player_ev = this->eval();
	}
	else {
		shoot_dealer_ev = shoot_dealer_live.expectimax() * probability_live +
		                  shoot_dealer_blank.expectimax() * probability_blank;
		shoot_player_ev = shoot_player_live.expectimax() * probability_live +
		                  shoot_player_blank.expectimax() * probability_blank;
	}

	if (shoot_dealer_ev >= shoot_player_ev && shoot_dealer_ev >= best_item_ev) {
		return std::pair<Action, float>(Action::SHOOT_DEALER, shoot_dealer_ev);
	}

	if (shoot_player_ev > shoot_dealer_ev && shoot_player_ev >= best_item_ev) {
		return std::pair<Action, float>(Action::SHOOT_PLAYER, shoot_player_ev);
	}

	return std::pair<Action, float>(best_item_action, best_item_ev);
}
