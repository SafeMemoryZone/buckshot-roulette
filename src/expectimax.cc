#include "expectimax.hpp"

#include <array>
#include <cassert>
#include <limits>

float heuristic_eval(Node node) {
	// TODO: Improve eval

	if (node.player_lives == 0) {
		return -100;
	}

	if (node.dealer_lives == 0) {
		return 100;
	}

	return (node.player_lives - node.dealer_lives) * 10;
}

inline Node new_node(bool is_dealer_turn, uint8_t live_round_count, uint8_t blank_round_count,
                     uint8_t max_lives, uint8_t dealer_lives, uint8_t player_lives,
                     std::vector<Action> available_dealer_item_actions,
                     std::vector<Action> available_player_item_actions) {
	return Node{is_dealer_turn,
	            live_round_count,
	            blank_round_count,
	            max_lives,
	            dealer_lives,
	            player_lives,
	            std::move(available_dealer_item_actions),
	            std::move(available_player_item_actions)};
}

inline bool is_only_live_rounds(Node node) {
	return node.live_round_count > 0 && node.blank_round_count == 0;
}

inline bool is_only_blank_rounds(Node node) {
	return node.blank_round_count > 0 && node.live_round_count == 0;
}

inline bool is_last_round(Node node) {
	return (node.live_round_count + node.blank_round_count) == 1;
}

inline bool is_terminal_node(Node node) {
	return node.dealer_lives == 0 || node.player_lives == 0 ||
	       (node.live_round_count + node.blank_round_count) == 0;
}

inline std::vector<Action> remove_item_action(
    const std::vector<Action>& curr_available_item_actions, int idx) {
	std::vector<Action> new_available_item_actions = curr_available_item_actions;
	new_available_item_actions.at(idx) = new_available_item_actions.back();
	new_available_item_actions.pop_back();
	return new_available_item_actions;
}

std::array<Node, 4> get_states_after_shoot(Node node) {
	Node shoot_dealer_blank =
	    new_node(true, node.live_round_count, static_cast<uint8_t>(node.blank_round_count - 1),
	             node.max_lives, node.dealer_lives, node.player_lives,
	             node.available_dealer_item_actions, node.available_player_item_actions);
	Node shoot_dealer_live = new_node(
	    !node.is_dealer_turn, static_cast<uint8_t>(node.live_round_count - 1),
	    node.blank_round_count, node.max_lives, static_cast<uint8_t>(node.dealer_lives - 1),
	    node.player_lives, node.available_dealer_item_actions, node.available_player_item_actions);
	Node shoot_player_blank =
	    new_node(false, node.live_round_count, static_cast<uint8_t>(node.blank_round_count - 1),
	             node.max_lives, node.dealer_lives, node.player_lives,
	             node.available_dealer_item_actions, node.available_player_item_actions);
	Node shoot_player_live =
	    new_node(!node.is_dealer_turn, static_cast<uint8_t>(node.live_round_count - 1),
	             node.blank_round_count, node.max_lives, node.dealer_lives,
	             static_cast<uint8_t>(node.player_lives - 1), node.available_dealer_item_actions,
	             node.available_player_item_actions);

	return {shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live};
}

float expectimax(Node node);

float drink_beer(Node node, float item_pickup_probability, int item_action_idx) {
	const float probability_live = static_cast<float>(node.live_round_count) /
	                               (node.live_round_count + node.blank_round_count);
	const float probability_blank = 1.0 - probability_live;

	if (is_only_live_rounds(node)) {
		Node eject_live = new_node(
		    node.is_dealer_turn, node.live_round_count - 1, node.blank_round_count, node.max_lives,
		    node.dealer_lives, node.player_lives,
		    node.is_dealer_turn
		        ? std::move(remove_item_action(node.available_dealer_item_actions, item_action_idx))
		        : node.available_dealer_item_actions,
		    !node.is_dealer_turn
		        ? std::move(remove_item_action(node.available_player_item_actions, item_action_idx))
		        : node.available_player_item_actions);
		return expectimax(eject_live) * item_pickup_probability;
	}
	if (is_only_blank_rounds(node)) {
		Node eject_blank = new_node(
		    node.is_dealer_turn, node.live_round_count, node.blank_round_count - 1, node.max_lives,
		    node.dealer_lives, node.player_lives,
		    node.is_dealer_turn
		        ? std::move(remove_item_action(node.available_dealer_item_actions, item_action_idx))
		        : node.available_dealer_item_actions,
		    !node.is_dealer_turn
		        ? std::move(remove_item_action(node.available_player_item_actions, item_action_idx))
		        : node.available_player_item_actions);
		return expectimax(eject_blank) * item_pickup_probability;
	}

	Node eject_live = new_node(
	    node.is_dealer_turn, node.live_round_count - 1, node.blank_round_count, node.max_lives,
	    node.dealer_lives, node.player_lives,
	    node.is_dealer_turn
	        ? std::move(remove_item_action(node.available_dealer_item_actions, item_action_idx))
	        : node.available_dealer_item_actions,
	    !node.is_dealer_turn
	        ? std::move(remove_item_action(node.available_player_item_actions, item_action_idx))
	        : node.available_player_item_actions);

	Node eject_blank = new_node(
	    node.is_dealer_turn, node.live_round_count, node.blank_round_count - 1, node.max_lives,
	    node.dealer_lives, node.player_lives,
	    node.is_dealer_turn
	        ? std::move(remove_item_action(node.available_dealer_item_actions, item_action_idx))
	        : node.available_dealer_item_actions,
	    !node.is_dealer_turn
	        ? std::move(remove_item_action(node.available_player_item_actions, item_action_idx))
	        : node.available_player_item_actions);

	return expectimax(eject_live) * item_pickup_probability * probability_live +
	       expectimax(eject_blank) * item_pickup_probability * probability_blank;
}

float smoke_cigarette(Node node, float item_pickup_probability, int item_action_idx) {
	return expectimax(new_node(
	           node.is_dealer_turn, node.live_round_count, node.blank_round_count, node.max_lives,
	           node.is_dealer_turn ? node.dealer_lives + 1 : node.dealer_lives,
	           !node.is_dealer_turn ? node.player_lives + 1 : node.player_lives,
	           node.is_dealer_turn
	               ? remove_item_action(node.available_dealer_item_actions, item_action_idx)
	               : node.available_dealer_item_actions,
	           !node.is_dealer_turn
	               ? remove_item_action(node.available_player_item_actions, item_action_idx)
	               : node.available_player_item_actions)) *
	       item_pickup_probability;
}

float expectimax(Node node) {
	if (is_terminal_node(node)) {
		return heuristic_eval(node);
	}

	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    get_states_after_shoot(node);
	const float probability_live = static_cast<float>(node.live_round_count) /
	                               (node.live_round_count + node.blank_round_count);
	const float probability_blank = 1.0 - probability_live;

	if (node.is_dealer_turn) {
		const float item_pickup_probability = 1.0 / node.available_dealer_item_actions.size();
		/*
		 * The dealer AI acts as follows:
		 * - It always knows the last round type and acts accordingly
		 * - If it doesn't know the currect round type, it flips a coin
		 * - Before shooting, it iterates through his items in the order they spawned (we assume the
		 * order is random) and decides if he wants to use them.
		 * Item usages:
		 * - Beer: If its not the last round and he the known round (if known) isn't live
		 */

		bool chose_action = false;
		float ev_after_item_usage = 0;

		for (int item_action_idx = 0; item_action_idx < node.available_dealer_item_actions.size();
		     item_action_idx++) {
			const Action item_action = node.available_dealer_item_actions.at(item_action_idx);

			switch (item_action) {
				case Action::DRINK_BEER:
					if (is_last_round(node)) break;
					ev_after_item_usage +=
					    drink_beer(node, item_pickup_probability, item_action_idx);
					chose_action = true;
					break;
				case Action::SMOKE_CIGARETTE:
					if (node.dealer_lives == node.max_lives) break;
					ev_after_item_usage +=
					    smoke_cigarette(node, item_pickup_probability, item_action_idx);
					chose_action = true;
					break;
				default:
					assert(false && "invalid item action");
			}
		}

		if (chose_action) {
			return ev_after_item_usage;
		}

		if (is_last_round(node)) {
			if (node.live_round_count == 1) {
				node.player_lives -= 1;
			}

			return heuristic_eval(node);
		}

		if (is_only_live_rounds(node)) {
			return expectimax(shoot_dealer_live) * 0.5 + expectimax(shoot_player_live) * 0.5;
		}

		if (is_only_blank_rounds(node)) {
			return expectimax(shoot_dealer_blank) * 0.5 + expectimax(shoot_player_blank) * 0.5;
		}

		return expectimax(shoot_dealer_live) * probability_live * 0.5 +
		       expectimax(shoot_dealer_blank) * probability_blank * 0.5 +
		       expectimax(shoot_player_live) * probability_live * 0.5 +
		       expectimax(shoot_player_blank) * probability_blank * 0.5;
	}

	float best_ev = std::numeric_limits<float>::lowest();

	// try items
	for (int item_action_idx = 0; item_action_idx < node.available_player_item_actions.size();
	     item_action_idx++) {
		const Action item_action = node.available_player_item_actions.at(item_action_idx);
		switch (item_action) {
			case Action::DRINK_BEER:
				best_ev = std::max(drink_beer(node, 1, item_action_idx), best_ev);
				break;
			case Action::SMOKE_CIGARETTE:
				if (node.player_lives == node.max_lives) break;
				best_ev = std::max(smoke_cigarette(node, 1, item_action_idx), best_ev);
				break;
			default:
				assert(false && "invalid item action");
		}
	}

	// player shoots
	if (is_only_live_rounds(node)) {
		best_ev = std::max(expectimax(shoot_dealer_live), best_ev);
	}
	else if (is_only_blank_rounds(node)) {
		// WARNING: This may not work if the eval function changes
		best_ev = std::max(heuristic_eval(node), best_ev);
	}
	else {
		best_ev = std::max(expectimax(shoot_dealer_live) * probability_live +
		                       expectimax(shoot_dealer_blank) * probability_blank,
		                   best_ev);
		best_ev = std::max(expectimax(shoot_player_live) * probability_live +
		                       expectimax(shoot_player_blank) * probability_blank,
		                   best_ev);
	}

	return best_ev;
}

std::pair<Action, float> get_best_action(Node node) {
	float best_item_ev = std::numeric_limits<float>::lowest();
	Action best_item_action;

	float shoot_player_ev = std::numeric_limits<float>::lowest();
	float shoot_dealer_ev = std::numeric_limits<float>::lowest();

	const float probability_live = static_cast<float>(node.live_round_count) /
	                               (node.live_round_count + node.blank_round_count);
	const float probability_blank = 1.0 - probability_live;
	auto [shoot_player_blank, shoot_player_live, shoot_dealer_blank, shoot_dealer_live] =
	    get_states_after_shoot(node);

	for (int item_action_idx = 0; item_action_idx < node.available_player_item_actions.size();
	     item_action_idx++) {
		const Action item_action = node.available_player_item_actions.at(item_action_idx);

		switch (item_action) {
			case Action::DRINK_BEER: {
				const float ev = drink_beer(node, 1, item_action_idx);
				if (ev > best_item_ev) {
					best_item_ev = ev;
					best_item_action = Action::DRINK_BEER;
				}
				break;
			}
			case Action::SMOKE_CIGARETTE: {
				if (node.player_lives == node.max_lives) break;
				const float ev = smoke_cigarette(node, 1, item_action_idx);
				if (ev > best_item_ev) {
					best_item_ev = ev;
					best_item_action = Action::SMOKE_CIGARETTE;
				}
				break;
			}
			default:
				assert(false && "invalid item action");
		}
	}

	if (is_only_live_rounds(node)) {
		shoot_dealer_ev = expectimax(shoot_dealer_live);
	}
	else if (is_only_blank_rounds(node)) {
		// WARNING: This may not work if the eval function changes
		shoot_player_ev = heuristic_eval(node);
	}
	else {
		shoot_dealer_ev = expectimax(shoot_dealer_live) * probability_live +
		                  expectimax(shoot_dealer_blank) * probability_blank;
		shoot_player_ev = expectimax(shoot_player_live) * probability_live +
		                  expectimax(shoot_player_blank) * probability_blank;
	}

	if (shoot_dealer_ev >= shoot_player_ev && shoot_dealer_ev >= best_item_ev) {
		return std::pair<Action, float>(Action::SHOOT_DEALER, shoot_dealer_ev);
	}

	if (shoot_player_ev > shoot_dealer_ev && shoot_player_ev >= best_item_ev) {
		return std::pair<Action, float>(Action::SHOOT_PLAYER, shoot_player_ev);
	}

	return std::pair<Action, float>(best_item_action, best_item_ev);
}
