#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "expectimax.hpp"
#include "item_manager.hpp"

struct Args {
	bool should_output_help = false;
};

void print_help(void) {
	std::cout << "Usage: buckshot-roulettee [FLAGS]\n"
	          << "  --help, --h        : Print this help message.\n"
	          << "  (No flags)         : Run the solver.\n";
}

Args parse_cmd_args(int argc, char **argv) {
	Args args;
	for (int i = 1; i < argc; ++i) {
		std::string curr = argv[i];
		if (curr == "--h" || curr == "--help") {
			args.should_output_help = true;
		}
		else {
			std::cerr << "[WARNING] Ignoring command line argument '" << curr << "'.\n";
		}
	}
	return args;
}

ItemManager prompt_items(std::string_view prompt) {
	std::cout << prompt << '\n';
	std::string curr_line;
	std::getline(std::cin, curr_line);

	ItemManager items;

	while (!curr_line.empty()) {
		if (curr_line == "beer") {
			items.add_beer();
		}
		else if (curr_line == "cigarettes") {
			items.add_cigarette_pack();
		}
		else if (curr_line == "magnifying glass") {
			items.add_magnifying_glass();
		}
		else if (curr_line == "handsaw") {
			items.add_handsaw();
		}
		else if (curr_line == "handcuffs") {
			items.add_handcuffs();
		}
		else {
			std::cout << "[ERROR] Unknown item name '" << curr_line
			          << "'\nAvailable items: beer, cigarettes, magnifying glass, handsaw and "
			             "handcuffs.\n";
		}
		std::getline(std::cin, curr_line);
	}

	return items;
}

template <typename... Args>
int prompt_num(int lower_bound, int upper_bound, Args &&...args) {
	while (true) {
		((std::cout << std::forward<Args>(args)), ...);
		int input;
		std::cin >> input;

		if (std::cin.fail() || input < lower_bound || input > upper_bound) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "[ERROR] Invalid input. Please enter a number between " << lower_bound
			          << " and " << upper_bound << ".\n";
		}
		else {
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return input;
		}
	}
}

bool prompt_is_live(std::string_view prompt) {
	std::cout << prompt;

	for (;;) {
		std::string c;
		std::cin >> c;
		if (c == "yes" || c == "y") {
			return true;
		}
		if (c == "no" || c == "n") {
			return false;
		}
		std::cout
		    << "[ERROR] Invalid input. Please use one of the following options: y, n, yes, no.\n";
	}
}

std::string action_to_str(Action action) {
	switch (action) {
		case Action::SHOOT_DEALER:
			return "shoot dealer";
		case Action::SHOOT_PLAYER:
			return "shoot player";
		case Action::DRINK_BEER:
			return "drink beer";
		case Action::SMOKE_CIGARETTE:
			return "smoke cigarette pack";
		case Action::USE_MAGNIFYING_GLASS:
			return "use magnifying glass";
		case Action::USE_HANDSAW:
			return "use handsaw";
		case Action::USE_HANDCUFFS:
			return "use handcuffs";
		default:
			assert(false);
	}
}

Action prompt_action(const std::vector<Action> &available_actions) {
	std::cout << "\n[PROMPT] Select an action for the dealer:\n";
	for (size_t i = 0; i < available_actions.size(); ++i) {
		std::cout << "  " << i + 1 << ". " << action_to_str(available_actions[i]) << '\n';
	}
	std::cout << "[PROMPT] Enter the number corresponding to the action: ";

	while (true) {
		int choice;
		std::cin >> choice;

		if (std::cin.fail() || choice < 1 || choice > static_cast<int>(available_actions.size())) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "[ERROR] Invalid choice. Please enter a number between 1 and "
			          << available_actions.size() << ": ";
		}
		else {
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return available_actions[choice - 1];
		}
	}
}

int main(int argc, char **argv) {
	Args args = parse_cmd_args(argc, argv);

	if (args.should_output_help) {
		print_help();
	}
	else {
		int round_num = prompt_num(1, 3, "[PROMPT] Enter current round number (1-3): ");

		uint8_t player_lives;
		uint8_t dealer_lives;
		uint8_t max_lives;

		switch (round_num) {
			case 1:
				dealer_lives = prompt_num(1, 2, "[PROMPT] Enter dealer lives (1-2): ");
				player_lives = prompt_num(1, 2, "[PROMPT] Enter player lives (1-2): ");
				max_lives = 2;
				break;
			case 2:
				dealer_lives = prompt_num(1, 4, "[PROMPT] Enter dealer lives (1-4): ");
				player_lives = prompt_num(1, 4, "[PROMPT] Enter player lives (1-4): ");
				max_lives = 4;
				break;
			case 3:
				dealer_lives = prompt_num(1, 6, "[PROMPT] Enter dealer lives (1-6): ");
				player_lives = prompt_num(1, 6, "[PROMPT] Enter player lives (1-6): ");
				max_lives = 6;
				break;
			default:
				assert(false);
		}

		uint8_t live_round_count = prompt_num(0, 8, "[PROMPT] Enter live round count (0-8): ");
		uint8_t blank_round_count =
		    prompt_num(live_round_count > 0 ? 0 : 1, 8 - live_round_count,
		               "[PROMPT] Enter blank round count (", (live_round_count > 0 ? "0" : "1"),
		               "-", std::to_string(8 - live_round_count), "): ");

		ItemManager dealer_items;
		ItemManager player_items;

		if (round_num > 1) {
			dealer_items = prompt_items("[PROMPT] Enter dealer items (end with an empty line): ");
			player_items = prompt_items("[PROMPT] Enter player items (end with an empty line): ");
		}

		Node node(false, false, false, live_round_count, blank_round_count, max_lives, dealer_lives,
		          player_lives, dealer_items, player_items);

		while (!node.is_terminal()) {
			std::cout << "[INFO] " << node.get_live_round_count() << " live rounds and "
			          << node.get_blank_round_count() << " blank rounds. Dealer has "
			          << node.get_dealer_lives() << " lives and player has "
			          << node.get_player_lives() << " lives.\n";
			if (node.is_player_turn()) {
				std::cout << "[INFO] It's the player's turn.\n";
				auto [best_action, ev] = node.get_best_action();

				std::string action_str = action_to_str(best_action);
				std::cout << "\n[INFO] Best action: " << action_str << " with eval " << ev << ".\n";

				switch (best_action) {
					case Action::SHOOT_DEALER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_shoot_dealer_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_shoot_dealer_blank();
							break;
						}
						bool is_live = prompt_is_live("[PROMPT] Player damaged the dealer (y/n): ");
						if (is_live) {
							node.apply_shoot_dealer_live();
						}
						else {
							node.apply_shoot_dealer_blank();
						}
					} break;
					case Action::SHOOT_PLAYER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_shoot_player_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_shoot_player_blank();
							break;
						}
						bool is_live = prompt_is_live("[PROMPT] Player damaged himself (y/n): ");
						if (is_live) {
							node.apply_shoot_player_live();
						}
						else {
							node.apply_shoot_player_blank();
						}
						break;
					}
					case Action::DRINK_BEER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_drink_beer_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_drink_beer_blank();
							break;
						}
						bool is_live =
						    prompt_is_live("[PROMPT] Player's beer ejected a live round (y/n): ");
						if (is_live) {
							node.apply_drink_beer_live();
						}
						else {
							node.apply_drink_beer_blank();
						}
						break;
					}
					case Action::SMOKE_CIGARETTE:
						node.apply_smoke_cigarette();
						break;
					case Action::USE_MAGNIFYING_GLASS: {
						bool is_live = prompt_is_live(
						    "[PROMPT] Player's magnifying glass showed a live round (y/n): ");
						if (is_live) {
							node.apply_magnify_live();
						}
						else {
							node.apply_magnify_blank();
						}
						break;
					}
					case Action::USE_HANDSAW:
						node.apply_use_handsaw();
						break;
					case Action::USE_HANDCUFFS:
						node.apply_use_handcuffs();
						break;
					default:
						assert(false);
				}
			}
			else {
				std::cout << "[INFO] It's the dealer's turn.\n";

				ItemManager dealer_items = node.get_dealer_items();
				std::vector<Action> dealer_available_actions = {Action::SHOOT_DEALER,
				                                                Action::SHOOT_PLAYER};

				if (dealer_items.has_beer()) {
					dealer_available_actions.emplace_back(Action::DRINK_BEER);
				}
				if (dealer_items.has_cigarette_pack()) {
					dealer_available_actions.emplace_back(Action::SMOKE_CIGARETTE);
				}
				if (dealer_items.has_magnifying_glass()) {
					dealer_available_actions.emplace_back(Action::USE_MAGNIFYING_GLASS);
				}
				if (dealer_items.has_handsaw()) {
					dealer_available_actions.emplace_back(Action::USE_HANDSAW);
				}
				if (dealer_items.has_handcuffs()) {
					dealer_available_actions.emplace_back(Action::USE_HANDCUFFS);
				}

				Action dealer_action = prompt_action(dealer_available_actions);

				switch (dealer_action) {
					case Action::SHOOT_DEALER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_shoot_dealer_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_shoot_dealer_blank();
							break;
						}
						bool is_live = prompt_is_live("[PROMPT] Dealer damaged himself (y/n): ");
						if (is_live) {
							node.apply_shoot_dealer_live();
						}
						else {
							node.apply_shoot_dealer_blank();
						}
					} break;
					case Action::SHOOT_PLAYER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_shoot_player_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_shoot_player_blank();
							break;
						}
						bool is_live = prompt_is_live("[PROMPT] Dealer damaged the player (y/n): ");
						if (is_live) {
							node.apply_shoot_player_live();
						}
						else {
							node.apply_shoot_player_blank();
						}
						break;
					}
					case Action::DRINK_BEER: {
						if (node.is_only_live_rounds() || node.round_known_live()) {
							node.apply_drink_beer_live();
							break;
						}
						if (node.is_only_blank_rounds() || node.round_known_blank()) {
							node.apply_drink_beer_blank();
							break;
						}
						bool is_live =
						    prompt_is_live("[PROMPT] Dealer's beer ejected a live round (y/n): ");
						if (is_live) {
							node.apply_drink_beer_live();
						}
						else {
							node.apply_drink_beer_blank();
						}
						break;
					}
					case Action::SMOKE_CIGARETTE:
						node.apply_smoke_cigarette();
						break;
					case Action::USE_MAGNIFYING_GLASS: {
						node.dealer_remove_magnifying_glass();
						break;
					}
					case Action::USE_HANDSAW:
						node.apply_use_handsaw();
						break;
					case Action::USE_HANDCUFFS:
						node.apply_use_handcuffs();
						break;
					default:
						assert(false);
				}
			}
		}
	}

	return 0;
}
