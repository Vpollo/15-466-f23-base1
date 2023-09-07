#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <map>

struct PuzzleMode : Mode {
	PuzzleMode();
	virtual ~PuzzleMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	// ----- game state -----
	bool playerCanMove = true;	// false when move anim playing

	//player position
	glm::vec2 player_pos = glm::vec2(0.0f);

	//input tracking.
	struct Button {
		bool triggered = false;
		bool released = true;
	} left, right, down, up;

	// ----- sprite tracking -----
	enum tile_type {
		player,
		normal_grid,
		reverse,
		key,
		door,
		block,
		up_arrow,
		down_arrow,
		left_arrow,
		right_arrow,
		background_tile
	};

	std::map<tile_type, uint8_t> tile_idx {
		{player, (uint8_t)0},
		{normal_grid, (uint8_t)1},
		{reverse, (uint8_t)2},
		{key, (uint8_t)3},
		{door, (uint8_t)4},
		{block, (uint8_t)5},
		{up_arrow, (uint8_t)6},
		{down_arrow, (uint8_t)7},
		{left_arrow, (uint8_t)8},
		{right_arrow, (uint8_t)9},
		{background_tile, (uint8_t)10}
	};

	std::map<tile_type, uint8_t> palette_idx;

	//----- helpers -----
	private:
	void load_sprite(const char* file_dir, tile_type ttype);
	// private:
	// void move_left();
	// void move_right();
	// void move_up();
	// void move_down();

	//----- drawing handled by PPU466 -----
	PPU466 ppu;

	//some weird background animation:
	float background_fade = 0.0f;
};