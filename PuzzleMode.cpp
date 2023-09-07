#include "PuzzleMode.hpp"

#include <fstream>
#include<iostream>

PuzzleMode::PuzzleMode() {
    //load all assets into PPU's memory
	//testing, palette and player only

	{   //Load palette
		std::ifstream palette_file("sprites-runtime/color_palette.bin", std::ios::binary | std::ios::in);
		assert(palette_file && "unable to read color_palette.bin");
		for (size_t i = 0; i < 8; i++) {
			palette_file.read(reinterpret_cast< char* >(&ppu.palette_table[i]), 16);
		}
	}
	

	//Load all sprites
	load_sprite("sprites-runtime/player.bin", player);
	load_sprite("sprites-runtime/normal_grid.bin", normal_grid);
	load_sprite("sprites-runtime/reverse_grid.bin", reverse);
	load_sprite("sprites-runtime/key_grid.bin", key);
	load_sprite("sprites-runtime/door_grid.bin", door);
	load_sprite("sprites-runtime/block_grid.bin", block);
	load_sprite("sprites-runtime/up_arrow.bin", up_arrow);
	load_sprite("sprites-runtime/left_arrow.bin", left_arrow);
	load_sprite("sprites-runtime/down_arrow.bin", down_arrow);
	load_sprite("sprites-runtime/right_arrow.bin", right_arrow);
	load_sprite("sprites-runtime/background.bin", background_tile);
	load_sprite("sprites-runtime/background_white.bin", background_white);
}

PuzzleMode::~PuzzleMode() {
}


bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_LEFT && left.released) {
			left.triggered = true;
			left.released = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT && right.released) {
			right.triggered = true;
			right.released = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP && up.released) {
			up.triggered = true;
			up.released = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN && down.released) {
			down.triggered = true;
			down.released = false;
			return true;
		}
    } else if (evt.type == SDL_KEYUP) {
        if (evt.key.keysym.sym == SDLK_LEFT) {
			left.released = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.released = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.released = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.released = true;
			return true;
		}
    }

    return false;
}

void PuzzleMode::update(float elapsed) {
    if (left.triggered) move_left();
    if (right.triggered) move_right();
    if (up.triggered) move_up();
    if (down.triggered) move_down();

    left.triggered = false;
    right.triggered = false;
    up.triggered = false;
    down.triggered = false;
}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	// draw every element in level grid individually
	uint8_t sprite_used = 0;
	draw_tile_of_type(door, sprite_used);
	draw_tile_of_type(key, sprite_used);
	draw_tile_of_type(reverse, sprite_used);
	draw_tile_of_type(block, sprite_used);
	draw_tile_of_type(player, sprite_used);

	// set all unused sprites off screen
	for (uint8_t i = sprite_used; i < 64; i++) {
		ppu.sprites[i].y = 255;
	}

	//Background tiles & color is just 3 * pure white pudding
	//and the center being LevelGridWidth*LevelGridHeight green grid
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			if ((y < 3) | (y > ((PPU466::BackgroundHeight / 2) - 4)) | (x < 3) | (x > ((PPU466::BackgroundWidth / 2) - 4))) {
				ppu.background[x+PPU466::BackgroundWidth*y] = tile_idx[background_white] | (palette_idx[background_white] << 8);
			} else {
				ppu.background[x+PPU466::BackgroundWidth*y] = tile_idx[background_tile] | (palette_idx[background_tile] << 8);
			}
		}
	}
	ppu.background_position.x = 0;
	ppu.background_position.y = 0;
	ppu.background_color = glm::u8vec4(0xff, 0xff, 0xff, 0xff);

	ppu.draw(drawable_size);
}

void PuzzleMode::move_left() {
    if (!playerCanMove) return;
	glm::u8vec2 *player_pos = &level_state[player][0];
	glm::u8vec2 player_pos_after = glm::u8vec2{player_pos->x - 1, player_pos->y};
	if (!can_move(player_pos_after)) return;

	*player_pos = player_pos_after;
}

void PuzzleMode::move_right() {
    if (!playerCanMove) return;
	glm::u8vec2 *player_pos = &level_state[player][0];
	glm::u8vec2 player_pos_after = glm::u8vec2{player_pos->x + 1, player_pos->y};
	if (!can_move(player_pos_after)) return;

	*player_pos = player_pos_after;
}

void PuzzleMode::move_up() {
    if (!playerCanMove) return;
	glm::u8vec2 *player_pos = &level_state[player][0];
	glm::u8vec2 player_pos_after = glm::u8vec2{player_pos->x, player_pos->y + 1};
	if (!can_move(player_pos_after)) return;

	*player_pos = player_pos_after;
}

void PuzzleMode::move_down() {
    if (!playerCanMove) return;
	glm::u8vec2 *player_pos = &level_state[player][0];
	glm::u8vec2 player_pos_after = glm::u8vec2{player_pos->x, player_pos->y - 1};
	if (!can_move(player_pos_after)) return;

	*player_pos = player_pos_after;
}

bool PuzzleMode::can_move(glm::u8vec2 pos){
	if ((pos.x < 0) || (pos.x >= PuzzleMode::LevelGridWidth) || 
	    (pos.y < 0) || (pos.y >= PuzzleMode::LevelGridHeight)) {
		return false;
	}

	std::vector<glm::u8vec2> *blocks = &level_state[block];
	// https://stackoverflow.com/questions/571394/how-to-find-out-if-an-item-is-present-in-a-stdvector
	if (std::find(blocks->begin(), blocks->end(), pos) != blocks->end()) {
		return false;
	}

	return true;
}

void PuzzleMode::load_sprite(const char* file_dir, tile_type ttype) {
	std::ifstream player_file(file_dir, std::ios::binary | std::ios::in);
	assert(player_file && "unable to read sprite");
	//read the palette index byte
	uint8_t palette_idx_;
	player_file.read(reinterpret_cast< char* >(&palette_idx_), 1);
	palette_idx[ttype] = palette_idx_;
	//load all 2-bit color index 4 at a time (1 byte)
	uint8_t buffer;
	int x, y;
	uint8_t idx = 0;
	PPU466::Tile *tile_writing = &ppu.tile_table[tile_idx[ttype]];
	for (uint8_t t = 0; t < 8; t++) {
		tile_writing->bit0[t] = (uint8_t)0;
		tile_writing->bit1[t] = (uint8_t)0;
	}
	for (size_t i = 0; i < 16; i++) {
		player_file.read(reinterpret_cast< char* >(&buffer), 1);
		for (size_t j = 0; j < 4; j++) {
			y = idx / 8;
			x = idx - y * 8;
			uint8_t bit_idx = (buffer >> ((3 - j) * 2)) & 0b11;
			uint8_t bit_idx_0 = bit_idx & 0b01;
			uint8_t bit_idx_1 = (bit_idx & 0b10) >> 1;
			tile_writing->bit0[y] = tile_writing->bit0[y] | (bit_idx_0 << x);
			tile_writing->bit1[y] = tile_writing->bit1[y] | (bit_idx_1 << x);
			idx++;
		}
	}
}

glm::u8vec2 PuzzleMode::grid_to_screen_pos(glm::u8vec2 grid_pos) {
	//Surrounding level grid is 3 tiles of white space padding
	uint8_t x = grid_pos.x + 3;
	uint8_t y = grid_pos.y + 3;
	return glm::uvec2(x * 8, y * 8);
}

void PuzzleMode::draw_tile_of_type(PuzzleMode::tile_type ttype, uint8_t &sprite_used) {
	if (sprite_used >= 64) return;
	for (const glm::u8vec2& p_pos : level_state[ttype]) {
		glm::u8vec2 screen_pos = grid_to_screen_pos(p_pos);
		ppu.sprites[sprite_used].x = screen_pos.x;
		ppu.sprites[sprite_used].y = screen_pos.y;
		ppu.sprites[sprite_used].index = tile_idx[ttype];
		ppu.sprites[sprite_used].attributes = palette_idx[ttype];
		sprite_used++;
	}
}