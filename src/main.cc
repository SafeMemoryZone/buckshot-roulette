#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "expectimax.hpp"

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
			std::cerr << "[WARNING] Ignoring command line argument '" << curr << "'\n";
		}
	}
	return args;
}

std::vector<Action> prompt_item_actions(std::string_view prompt) {
	std::cout << prompt << '\n';
	std::string curr_line;
	std::getline(std::cin, curr_line);

	static const std::unordered_map<std::string, Action> action_map = {
	    {"beer", Action::DRINK_BEER},
	    {"cigarette", Action::SMOKE_CIGARETTE},
	    {"magnifying glass", Action::USE_MAGNIFYING_GLASS}};

	std::vector<Action> item_actions;

	while (!curr_line.empty()) {
		auto it = action_map.find(curr_line);
		if (it != action_map.end()) {
			item_actions.emplace_back(it->second);
			if (item_actions.size() == 8) break;
		}
		else {
			std::cout << "[ERROR] Unknown item name '" << curr_line
			          << "'\nAvailable items: beer, cigarette, magnifying glass\n";
		}
		std::getline(std::cin, curr_line);
	}

	return item_actions;
}

template <typename... Args>
int prompt_num(int lower_bound, int upper_bound, Args &&...args) {
	uint8_t num = 0;

	while (true) {
		((std::cout << std::forward<Args>(args)), ...);
		int input;
		std::cin >> input;

		if (std::cin.fail() || input < lower_bound || input > upper_bound) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "[ERROR] Invalid input. Please enter a number between " << lower_bound
			          << " and " << upper_bound << '\n';
		}
		else {
			num = input;
			break;
		}
	}

	return num;
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
				player_lives = prompt_num(1, 2, "[PROMPT] Enter player lives (1-2): ");
				dealer_lives = prompt_num(1, 2, "[PROMPT] Enter dealer lives (1-2): ");
				max_lives = 2;
				break;
			case 2:
				player_lives = prompt_num(1, 4, "[PROMPT] Enter player lives (1-4): ");
				dealer_lives = prompt_num(1, 4, "[PROMPT] Enter dealer lives (1-4): ");
				max_lives = 4;
				break;
			case 3:
				player_lives = prompt_num(1, 6, "[PROMPT] Enter player lives (1-6): ");
				dealer_lives = prompt_num(1, 6, "[PROMPT] Enter dealer lives (1-6): ");
				max_lives = 6;
				break;
			default:
				assert(false);
		}

		uint8_t live_round_count = prompt_num(0, 8, "[PROMPT] Enter live round count (0-8): ");
		uint8_t blank_round_count = prompt_num(live_round_count > 0 ? 0 : 1, 8 - live_round_count,
		                                       "[PROMPT] Enter blank round count (0-8): ");

		std::vector<Action> dealer_items = prompt_item_actions("[PROMPT] Enter dealer items: ");
		std::vector<Action> player_items = prompt_item_actions("[PROMPT] Enter player items: ");

		Node node(false, false, false, live_round_count, blank_round_count, max_lives, dealer_lives,
		          player_lives, dealer_items, player_items);

		while (!node.is_terminal()) {
			auto [best_action, ev] = node.get_best_action();
		}
	}

	return 0;
}
