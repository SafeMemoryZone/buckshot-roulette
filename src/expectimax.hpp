#ifndef EXPECTIMAX_HPP
#define EXPECTIMAX_HPP
#include <cstdint>
#include <vector>

enum class Action {
	SHOOT_DEALER,
	SHOOT_PLAYER,
	DRINK_BEER,
	SMOKE_CIGARETTE,
	USE_MAGNIFYING_GLASS,
};

class Node {
   public:
	explicit Node(bool is_dealer_turn, bool curr_is_live, bool curr_is_blank,
	              uint8_t live_round_count, uint8_t blank_round_count, uint8_t max_lives,
	              uint8_t dealer_lives, uint8_t player_lives,
	              const std::vector<Action> &available_dealer_item_actions,
	              const std::vector<Action> &available_player_item_actions);

	std::pair<Action, float> get_best_action(void) const;
	bool is_terminal(void) const;
	void apply_shoot_dealer_live(void);
	void apply_shoot_dealer_blank(void);
	void apply_shoot_player_live(void);
	void apply_shoot_player_blank(void);
    void apply_drink_beer_live(void);
    void apply_drink_beer_blank(void);
    void apply_smoke_cigarette(void);
    void apply_magnify_live(void);
    void apply_magnify_blank(void);

   private:
	float expectimax(void) const;
	float eval(void) const;
	bool is_only_live_rounds(void) const;
	bool is_only_blank_rounds(void) const;
	bool is_last_round(void) const;
	std::array<Node, 4> get_states_after_shoot(void) const;
	float drink_beer(float item_pickup_probability, int item_action_idx) const;
	float smoke_cigarette(float item_pickup_probability, int item_action_idx) const;
	float use_magnifying_glass(float item_pickup_probability, int item_action_idx) const;
    int find_item_action_idx(Action item_action) const;
    void remove_dealer_item_action(int item_action_idx);
    void remove_player_item_action(int item_action_idx);

	friend float eval_expectimax_performance(int sim_count, uint8_t live_round_count,
	                                         uint8_t blank_round_count, uint8_t max_lives,
	                                         std::vector<Action> &available_item_actions);

	bool is_dealer_turn : 1;
	bool curr_is_live : 1;
	bool curr_is_blank : 1;
	uint8_t live_round_count : 4;
	uint8_t blank_round_count : 4;
	uint8_t max_lives : 4;
	uint8_t dealer_lives : 4;
	uint8_t player_lives : 4;
	std::vector<Action> available_dealer_item_actions;
	std::vector<Action> available_player_item_actions;
};

#endif
