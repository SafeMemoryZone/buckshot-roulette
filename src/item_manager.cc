#include "item_manager.hpp"

#include <cassert>

#define MAGNIFYING_GLASS_SHIFT 0
#define CIGARETTE_PACK_SHIFT 4
#define BEER_SHIFT 8
#define HANDSAW_SHIFT 12
#define HANDCUFF_SHIFT 16

ItemManager::ItemManager(int magnifying_glass_count, int cigarette_pack_count, int beer_count,
                         int handsaw_count, int handcuff_count) {
	// MSB
	// 19-16: handcuff count
	// 15-12: handsaw count
	// 11-8: beer count
	// 7-4: cigarette pack count
	// 3-0: magnifying glass count
	// LSB
	assert(magnifying_glass_count > 0 && magnifying_glass_count <= 8);
	assert(cigarette_pack_count > 0 && cigarette_pack_count <= 8);
	assert(beer_count > 0 && beer_count <= 8);
	assert(handsaw_count > 0 && handsaw_count <= 8);
	assert(handcuff_count > 0 && handcuff_count <= 8);

	this->items = magnifying_glass_count << MAGNIFYING_GLASS_SHIFT |
	              cigarette_pack_count << CIGARETTE_PACK_SHIFT | beer_count << BEER_SHIFT |
	              handsaw_count << HANDSAW_SHIFT | handcuff_count << HANDCUFF_SHIFT;
}

bool ItemManager::has_magnifying_glass(void) const {
	return (this->items >> MAGNIFYING_GLASS_SHIFT & 0xF) > 0;
}

bool ItemManager::has_cigarette_pack(void) const {
	return (this->items >> CIGARETTE_PACK_SHIFT & 0xF) > 0;
}

bool ItemManager::has_beer(void) const { return (this->items >> BEER_SHIFT & 0xF) > 0; }

bool ItemManager::has_handsaw(void) const { return (this->items >> HANDSAW_SHIFT & 0xF) > 0; }

bool ItemManager::has_handcuffs(void) const { return (this->items >> HANDCUFF_SHIFT & 0xF) > 0; }

void ItemManager::remove_magnifying_glass(void) {
	int magnifying_glass_count = this->items >> MAGNIFYING_GLASS_SHIFT & 0xF;
	assert(magnifying_glass_count > 0);
	magnifying_glass_count--;
	uint32_t mask = ~(0xF << MAGNIFYING_GLASS_SHIFT);
	this->items &= mask;
	this->items |= magnifying_glass_count << MAGNIFYING_GLASS_SHIFT;
}

void ItemManager::remove_cigarette_pack(void) {
	int cigarette_pack_count = this->items >> CIGARETTE_PACK_SHIFT & 0xF;
	assert(cigarette_pack_count > 0);
	cigarette_pack_count--;
	uint32_t mask = ~(0xF << CIGARETTE_PACK_SHIFT);
	this->items &= mask;
	this->items |= cigarette_pack_count << CIGARETTE_PACK_SHIFT;
}

void ItemManager::remove_beer(void) {
	int beer_count = this->items >> BEER_SHIFT & 0xF;
	assert(beer_count > 0);
	beer_count--;
	uint32_t mask = ~(0xF << BEER_SHIFT);
	this->items &= mask;
	this->items |= beer_count << BEER_SHIFT;
}

void ItemManager::remove_handsaw(void) {
	int handsaw_count = this->items >> HANDSAW_SHIFT & 0xF;
	assert(handsaw_count > 0);
	handsaw_count--;
	uint32_t mask = ~(0xF << HANDSAW_SHIFT);
	this->items &= mask;
	this->items |= handsaw_count << HANDSAW_SHIFT;
}

void ItemManager::remove_handcuffs(void) {
	int handcuff_count = this->items >> HANDCUFF_SHIFT & 0xF;
	assert(handcuff_count > 0);
	handcuff_count--;
	uint32_t mask = ~(0xF << HANDCUFF_SHIFT);
	this->items &= mask;
	this->items |= handcuff_count << HANDCUFF_SHIFT;
}

int ItemManager::get_item_count(void) const {
	return (this->items >> MAGNIFYING_GLASS_SHIFT & 0xF) +
	       (this->items >> CIGARETTE_PACK_SHIFT & 0xF) + (this->items >> BEER_SHIFT & 0xF) +
	       (this->items >> HANDSAW_SHIFT & 0xF) + (this->items >> HANDCUFF_SHIFT & 0xF);
}
