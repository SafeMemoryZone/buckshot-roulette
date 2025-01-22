#ifndef ITEM_MANAGER_HPP
#define ITEM_MANAGER_HPP
#include <cstdint>

class ItemManager final {
   public:
	explicit ItemManager(int magnifying_glass_count, int cigarette_pack_count, int beer_count,
	                     int handsaw_count, int handcuff_count);

    ItemManager() = default;

	bool has_magnifying_glass(void) const;
	bool has_cigarette_pack(void) const;
	bool has_beer(void) const;
	bool has_handsaw(void) const;
	bool has_handcuffs(void) const;

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
	uint32_t items = 0;
};
#endif  // ITEM_MANAGER_HPP
