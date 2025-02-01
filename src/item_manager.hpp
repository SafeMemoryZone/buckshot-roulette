#ifndef ITEM_MANAGER_HPP
#define ITEM_MANAGER_HPP
#include <cstddef>
#include <cstdint>
#include <functional>

class Node;

class ItemManager final {
   public:
	explicit ItemManager(int magnifying_glass_count, int cigarette_pack_count, int beer_count,
	                     int handsaw_count, int handcuff_count);

	ItemManager() = default;

	ItemManager(const ItemManager &) = default;
	ItemManager(ItemManager &&) = default;
	ItemManager &operator=(const ItemManager &) = default;
	ItemManager &operator=(ItemManager &&) = default;
	bool operator==(const ItemManager &other) const;

	bool has_magnifying_glass(void) const;
	bool has_cigarette_pack(void) const;
	bool has_beer(void) const;
	bool has_handsaw(void) const;
	bool has_handcuffs(void) const;

	bool get_magnifying_glass_count(void) const;
	bool get_cigarette_pack_count(void) const;
	bool get_beer_count(void) const;
	bool get_handsaw_count(void) const;
	bool get_handcuffs_count(void) const;

	void remove_magnifying_glass(void);
	void remove_cigarette_pack(void);
	void remove_beer(void);
	void remove_handsaw(void);
	void remove_handcuffs(void);

	void add_magnifying_glass(void);
	void add_cigarette_pack(void);
	void add_beer(void);
	void add_handsaw(void);
	void add_handcuffs(void);

	int get_item_count(void) const;

   private:
	friend struct std::hash<Node>;

	uint32_t items = 0;
};
#endif  // ITEM_MANAGER_HPP
