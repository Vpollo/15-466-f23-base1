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
    // if (left.triggered) move_left();
    // if (right.triggered) move_right();
    // if (up.triggered) move_up();
    // if (down.triggered) move_down();

    left.triggered = false;
    right.triggered = false;
    up.triggered = false;
    down.triggered = false;

	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);
}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	//try to draw player
	ppu.sprites[63].x = 50;
	ppu.sprites[63].y = 150;
	ppu.sprites[63].index = tile_idx[player];
	ppu.sprites[63].attributes = 0b00000000;

	for (uint32_t i = 0; i < 63; ++i) {
		ppu.sprites[i].y = 241;
	}

	// //background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
		}
	}

	ppu.draw(drawable_size);
}

// void PuzzleMode::move_left() {
//     if (!playerCanMove) return;
// }

// void PuzzleMode::move_right() {
//     if (!playerCanMove) return;
// }

// void PuzzleMode::move_up() {
//     if (!playerCanMove) return;
// }

// void PuzzleMode::move_down() {
//     if (!playerCanMove) return;
// }

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