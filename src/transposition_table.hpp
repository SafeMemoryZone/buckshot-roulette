#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP
#include <cstddef>
#include <functional>
#include <optional>
#include <unordered_map>

#include "expectimax.hpp"

template <>
struct std::hash<Node> {
	std::size_t operator()(const Node &node) const;
};

constexpr int TRANSPOSITION_TABLE_MAX_SIZE = 65'535;

class TranspositionTableManager {
   public:
	TranspositionTableManager() = default;

	void add_node(const Node &node, float ev);
	std::optional<float> get_ev(const Node &node);
    void clear_table(void);

   private:
	std::unordered_map<Node, float> transposition_table;
};

#endif
