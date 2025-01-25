#include "expectimax.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <optional>

#include "transposition_table.hpp"

TranspositionTableManager tt_manager;

Node::Node(bool is_dealer_turn, bool curr_is_live, bool curr_is_blank, uint8_t live_round_count,
           uint8_t blank_round_count, uint8_t max_lives, uint8_t dealer_lives, uint8_t player_lives,
           ItemManager dealer_items, ItemManager player_items)
    : is_dealer_turn(is_dealer_turn),
      curr_is_live(curr_is_live),
      curr_is_blank(curr_is_blank),
      live_round_count(live_round_count),
      blank_round_count(blank_round_count),
      max_lives(max_lives),
      dealer_lives(dealer_lives),
      player_lives(player_lives),
      dealer_items(dealer_items),
      player_items(player_items) {}

bool Node::operator==(const Node &other) const {
	return this->dealer_items == other.dealer_items && this->player_items == other.player_items &&
	       this->live_round_count == other.live_round_count &&
	       this->blank_round_count == other.blank_round_count &&
	       this->max_lives == other.max_lives && this->dealer_lives == other.dealer_lives &&
	       this->player_lives == other.player_lives &&
	       this->is_dealer_turn == other.is_dealer_turn &&
	       this->curr_is_live == other.curr_is_live && this->curr_is_blank == other.curr_is_blank;
}

void Node::apply_shoot_dealer_live(void) {
	assert(this->dealer_lives > 0);
	assert(this->live_round_count > 0);

	this->dealer_lives--;
	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = !this->is_dealer_turn;
}

void Node::apply_shoot_dealer_blank(void) {
	assert(this->blank_round_count > 0);

	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = true;
}

void Node::apply_shoot_player_live(void) {
	assert(this->player_lives > 0);
	assert(this->live_round_count > 0);

	this->player_lives--;
	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = !this->is_dealer_turn;
}

void Node::apply_shoot_player_blank(void) {
	assert(blank_round_count > 0);

	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;
	this->is_dealer_turn = false;
}

void Node::apply_drink_beer_live(void) {
	assert(this->live_round_count > 0);

	this->live_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;

	if (this->is_dealer_turn) {
		this->dealer_items.remove_beer();
	}
	else {
		this->player_items.remove_beer();
	}
}

void Node::apply_drink_beer_blank(void) {
	this->blank_round_count--;
	this->curr_is_live = false;
	this->curr_is_blank = false;

	if (this->is_dealer_turn) {
		this->dealer_items.remove_beer();
	}
	else {
		this->player_items.remove_beer();
	}
}

void Node::apply_smoke_cigarette(void) {
	if (this->is_dealer_turn) {
		assert(this->dealer_lives < this->max_lives);
		this->dealer_lives++;
		this->dealer_items.remove_cigarette_pack();
	}
	else {
		assert(this->player_lives < this->max_lives);
		this->player_lives++;
		this->player_items.remove_cigarette_pack();
	}
}

void Node::apply_magnify_live(void) {
	this->curr_is_live = true;
	this->curr_is_blank = false;

	if (this->is_dealer_turn) {
		this->dealer_items.remove_magnifying_glass();
	}
	else {
		this->player_items.remove_magnifying_glass();
	}
}

void Node::apply_magnify_blank(void) {
	this->curr_is_live = false;
	this->curr_is_blank = true;

	if (this->is_dealer_turn) {
		this->dealer_items.remove_magnifying_glass();
	}
	else {
		this->player_items.remove_magnifying_glass();
	}
}

std::array<Node, 4> Node::get_states_after_shoot(void) const {
	Node shoot_dealer_live = *this;
	Node shoot_player_live = *this;
	Node shoot_dealer_blank = *this;
	Node shoot_player_blank = *this;

	if (this->live_round_count > 0 && this->dealer_lives > 0) {
		shoot_dealer_live.apply_shoot_dealer_live();
	}
	if (this->live_round_count > 0 && this->player_lives > 0) {
		shoot_player_live.apply_shoot_player_live();
	}
	if (this->blank_round_count > 0) {
		shoot_dealer_blank.apply_shoot_dealer_blank();
		shoot_player_blank.apply_shoot_player_blank();
	}

	return {shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live};
}

float Node::calc_drink_beer_ev(float item_pickup_probability) const {
	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;

	Node eject_live = *this;
	Node eject_blank = *this;

	if (this->is_only_live_rounds()) {
		eject_live.apply_drink_beer_live();
		return eject_live.expectimax() * item_pickup_probability;
	}
	if (this->is_only_blank_rounds()) {
		eject_blank.apply_drink_beer_blank();
		return eject_blank.expectimax() * item_pickup_probability;
	}

	eject_live.apply_drink_beer_live();
	eject_blank.apply_drink_beer_blank();

	return eject_live.expectimax() * probability_live * item_pickup_probability +
	       eject_blank.expectimax() * probability_blank * item_pickup_probability;
}

float Node::calc_smoke_cigarette_ev(float item_pickup_probability) const {
	Node smoked = *this;
	smoked.apply_smoke_cigarette();
	return smoked.expectimax() * item_pickup_probability;
}

float Node::calc_use_magnifying_glass_ev(float item_pickup_probability) const {
	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;

	Node magnify_live = *this;
	Node magnify_blank = *this;

	if (this->is_only_live_rounds()) {
		magnify_live.apply_magnify_live();
		return magnify_live.expectimax() * item_pickup_probability;
	}
	if (this->is_only_blank_rounds()) {
		magnify_blank.apply_magnify_blank();
		return magnify_blank.expectimax() * item_pickup_probability;
	}

	magnify_live.apply_magnify_live();
	magnify_blank.apply_magnify_blank();

	return magnify_live.expectimax() * probability_live * item_pickup_probability +
	       magnify_blank.expectimax() * probability_blank * item_pickup_probability;
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

float Node::expectimax(void) const {
	if (this->is_terminal()) {
		return this->eval();
	}

	if (std::optional<float> ev = tt_manager.get_ev(*this)) {
		std::cout << "Hit\n";
		return ev.value();
	}

	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;
	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    this->get_states_after_shoot();

	if (this->is_dealer_turn) {
		const float item_pickup_probability = 1.0f / this->dealer_items.get_item_count();
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

		bool chose_item = false;
		float ev_after_item_usage = 0.0f;

		if (this->dealer_items.has_beer() && !this->curr_is_live && !this->is_last_round()) {
			ev_after_item_usage += this->calc_drink_beer_ev(item_pickup_probability);
			chose_item = true;
		}
		if (this->dealer_items.has_cigarette_pack() && this->dealer_lives != this->max_lives) {
			ev_after_item_usage += this->calc_smoke_cigarette_ev(item_pickup_probability);
			chose_item = true;
		}
		if (this->dealer_items.has_magnifying_glass() && !this->curr_is_live &&
		    !this->curr_is_blank && !this->is_last_round()) {
			ev_after_item_usage += this->calc_use_magnifying_glass_ev(item_pickup_probability);
			chose_item = true;
		}

		if (chose_item) {
			tt_manager.add_node(*this, ev_after_item_usage);
			return ev_after_item_usage;
		}

		if (this->is_last_round()) {
			if (this->live_round_count == 1) {
				return shoot_player_live.eval();
			}

			return shoot_dealer_blank.eval();
		}

		if (this->is_only_live_rounds()) {
			const float ev =
			    shoot_dealer_live.expectimax() * 0.5f + shoot_player_live.expectimax() * 0.5f;
			tt_manager.add_node(*this, ev);
			return ev;
		}

		if (this->is_only_blank_rounds()) {
			const float ev =
			    shoot_dealer_blank.expectimax() * 0.5f + shoot_player_blank.expectimax() * 0.5f;
			tt_manager.add_node(*this, ev);
			return ev;
		}

		const float ev = shoot_dealer_live.expectimax() * probability_live * 0.5f +
		                 shoot_dealer_blank.expectimax() * probability_blank * 0.5f +
		                 shoot_player_live.expectimax() * probability_live * 0.5f +
		                 shoot_player_blank.expectimax() * probability_blank * 0.5f;
		tt_manager.add_node(*this, ev);
		return ev;
	}

	float best_ev = std::numeric_limits<float>::lowest();

	if (this->player_items.has_beer() && !this->curr_is_blank && !this->is_only_blank_rounds()) {
		best_ev = std::max(this->calc_drink_beer_ev(1.0f), best_ev);
	}
	if (this->player_items.has_cigarette_pack() && this->dealer_lives != this->max_lives) {
		best_ev = std::max(this->calc_smoke_cigarette_ev(1.0f), best_ev);
	}
	if (this->player_items.has_magnifying_glass() && !this->curr_is_live && !this->curr_is_blank &&
	    !this->is_only_live_rounds() && !this->is_only_blank_rounds()) {
		best_ev = std::max(this->calc_use_magnifying_glass_ev(1.0f), best_ev);
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

	tt_manager.add_node(*this, best_ev);
	return best_ev;
}

std::pair<Action, float> Node::get_best_action(void) const {
	float best_item_ev = std::numeric_limits<float>::lowest();
	Action best_action = Action::SHOOT_DEALER;

	float shoot_player_ev = std::numeric_limits<float>::lowest();
	float shoot_dealer_ev = std::numeric_limits<float>::lowest();

	const float probability_live = static_cast<float>(this->live_round_count) /
	                               (this->live_round_count + this->blank_round_count);
	const float probability_blank = 1.0f - probability_live;
	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    this->get_states_after_shoot();

	if (this->player_items.has_beer() && !this->curr_is_blank && !this->is_only_blank_rounds()) {
		const float ev = this->calc_drink_beer_ev(1.0f);
		if (ev > best_item_ev) {
			best_item_ev = ev;
			best_action = Action::DRINK_BEER;
		}
	}
	if (this->player_items.has_cigarette_pack() && this->dealer_lives != this->max_lives) {
		const float ev = this->calc_smoke_cigarette_ev(1.0f);
		if (ev > best_item_ev) {
			best_item_ev = ev;
			best_action = Action::SMOKE_CIGARETTE;
		}
	}
	if (this->player_items.has_magnifying_glass() && !this->curr_is_live && !this->curr_is_blank &&
	    !this->is_only_live_rounds() && !this->is_only_blank_rounds()) {
		const float ev = this->calc_use_magnifying_glass_ev(1.0f);
		if (ev > best_item_ev) {
			best_item_ev = ev;
			best_action = Action::USE_MAGNIFYING_GLASS;
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

	return std::pair<Action, float>(best_action, best_item_ev);
}
