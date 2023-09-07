from PIL import Image
import struct

#Color palette binary:
# - every four bytes = 1 color palette (rgba)
# - 4 * 8 bytes total, corresponding to 8 color palettes allocatable

def write_sprite_to_binary():
    # ----- load color palette into dict & write to binary -----
    palette_png = Image.open('../sprites/color_palette.png')
    assert(palette_png.size == (4,8))
    palette_pix = palette_png.load()

    palette_f = open('../sprites-runtime/color_palette.bin', 'wb')
    palettes = []
    for y in range (palette_png.size[1]):
        # store individual palette into dict for easy lookup when parsing sprites
        palette = {}
        for x in range(palette_png.size[0]):
            pix = palette_pix[x, y]
            palette[pix] = x
            # write rgba byte to binary
            for i in range(4):
                pix_byte = struct.pack('B', pix[i])
                palette_f.write(pix_byte)
        palettes.append(palette)

    # ----- Load all png sprites and convert them into 17-byte run time format -----
    # testing, just the player
    sprite = Image.open('../sprites/player.png')
    assert(sprite.size == (8,8))
    sprite_pix = sprite.load()

    # find the color palette that contain all colors in sprite
    # !!! really stupid code warning !!! (but it works XD)
    colors = []
    palette = {}
    palette_id = 0
    for y in range(0, sprite.size[1]):
        for x in range(0, sprite.size[0]):
            pix = sprite_pix[x, y]
            if pix not in colors:
                colors.append(pix)
    for palette_ in palettes:
        color_in_dict = palette_.keys()
        color_matched = 0
        for color in colors:
            if color in color_in_dict:
                color_matched += 1
        if color_matched == len(colors):
            palette = palette_
            break
        palette_id += 1
    assert(not palette == {})

    # write sprite as 8*8 2 bit color index runtime format
    # we increment by 4 in inner for loop bc we want to wait for 
    #  a whole byte to write to binary.
    sprite_rt_f = open('../sprites-runtime/player.bin', 'wb')
    sprite_rt_f.write(struct.pack('B', palette_id))
    for y in range(sprite.size[1]-1, -1, -1):
        for x in range(0, sprite.size[0], 4):
            byte_to_write : int = 0
            for i in range(4):
                bit_map = palette[sprite_pix[x+i,y]]
                byte_to_write = byte_to_write | (bit_map << (3-i) * 2)
            sprite_rt_f.write(struct.pack('B', byte_to_write))

if __name__ == "__main__":
    write_sprite_to_binary()