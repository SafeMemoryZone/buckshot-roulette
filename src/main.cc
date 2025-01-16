#include <iostream>

#include "test.hpp"

int main(void) {
	std::vector<Action> actions = {Action::SMOKE_CIGARETTE, Action::DRINK_BEER, Action::USE_MAGNIFYING_GLASS};
	std::cout << "Accuracy: " << eval_expectimax_performance(10, 4, 4, 6, actions) << '\n';
}
