#include "transposition_table.hpp"

#include <optional>

std::size_t std::hash<Node>::operator()(const Node &node) const {
	return static_cast<std::size_t>(node.dealer_items.items & 0xFFFFF) |
	       (static_cast<std::size_t>(node.player_items.items & 0xFFFFF) << 20) |
	       (static_cast<std::size_t>(node.live_round_count & 0xF) << 40) |
	       (static_cast<std::size_t>(node.blank_round_count & 0b111) << 44) |
	       (static_cast<std::size_t>(node.max_lives & 0b111) << 47) |
	       (static_cast<std::size_t>(node.dealer_lives & 0b111) << 50) |
	       (static_cast<std::size_t>(node.player_lives & 0b111) << 53) |
	       (static_cast<std::size_t>(node.is_dealer_turn & 0b1) << 56) |
	       (static_cast<std::size_t>(node.curr_is_live & 0b1) << 57) |
	       (static_cast<std::size_t>(node.curr_is_blank & 0b1) << 58) |
	       (static_cast<std::size_t>(node.handsaw_applied & 0b1) << 59) |
	       (static_cast<std::size_t>(node.handcuffs_applied & 0b1) << 60) |
	       (static_cast<std::size_t>(node.handcuffs_available & 0b1) << 61);
}

void TranspositionTableManager::add_node(const Node &node, float ev) {
	if (this->transposition_table.size() == TRANSPOSITION_TABLE_MAX_SIZE) {
		auto it = this->transposition_table.begin();
		this->transposition_table.erase(it);
	}

	this->transposition_table[node] = ev;
}

std::optional<float> TranspositionTableManager::get_ev(const Node &node) {
	auto match = this->transposition_table.find(node);
	if (match != this->transposition_table.end()) {
		return match->second;
	}
	return std::nullopt;
}

void TranspositionTableManager::clear_table(void) { this->transposition_table.clear(); }
