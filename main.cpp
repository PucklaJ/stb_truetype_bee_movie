#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

constexpr char default_font_file[] = "/usr/share/fonts/TTF/DejaVuSans.ttf";
constexpr auto default_font_size = 25;
constexpr char default_output_file_name[] = "output.txt";

struct FontBitmap {
  std::vector<unsigned char> pixels;
  size_t pixel_width;
  size_t pixel_height;
};

struct StringBitmap {
  std::vector<FontBitmap> bitmaps;
  std::vector<size_t> x_positions;
  std::vector<long> y_positions;
};

// Arguments:
// argv[1] ..... font size in pixel
// argv[2] ..... font file
// argv[3] ..... string which will be written instead of bee movie
// argv[4] ..... output file
//
// All arguments are optional
//
int main(int args, char *argv[]) {
  // Variables for all the arguments
  float font_size;
  std::string font_file_name;
  std::string input_string;
  std::string output_file_name;

  // Parse the arguments
  if (args >= 5) {
    output_file_name = argv[4];
  } else {
    output_file_name = default_output_file_name;
  }
  if (args >= 4) {
    input_string = argv[3];
  } else {
    // Parse bee move script
    std::string bee_movie_text;
    std::ifstream bee_movie_file("bee_movie.txt", std::ios_base::ate);
    if (bee_movie_file.fail()) {
      std::cerr << "Failed to open bee movie script" << std::endl;
      return 1;
    }
    const auto bee_move_file_size = bee_movie_file.tellg();
    bee_movie_text.resize(bee_move_file_size);
    bee_movie_file.seekg(0);
    bee_movie_file.read((char *)bee_movie_text.data(), bee_move_file_size);
    bee_movie_file.close();
    // Remove all \0 characters at the end
    while (bee_movie_text.back() == '\0') {
      bee_movie_text.resize(bee_movie_text.length() - 1);
    }
    std::cout << "Bee Movie Script: " << bee_movie_text.length()
              << " Characters" << std::endl;
    input_string = std::move(bee_movie_text);
  }
  if (args >= 3) {
    font_file_name = argv[2];
  } else {
    font_file_name = default_font_file;
  }
  if (args >= 2) {
    std::stringstream converter;
    converter << argv[1];
    converter >> font_size;
  } else {
    font_size = default_font_size;
  }

  stbtt_fontinfo font;
  std::vector<unsigned char> font_buffer;

  // Read font file
  std::ifstream font_file(font_file_name,
                          std::ios_base::binary | std::ios_base::ate);
  if (font_file.fail()) {
    std::cerr << "Failed to open font file" << std::endl;
    return 1;
  }
  const auto font_file_size = font_file.tellg();
  font_file.seekg(0);
  font_buffer.resize(font_file_size);
  font_file.read(reinterpret_cast<char *>(font_buffer.data()), font_file_size);
  font_file.close();

  // Load font file
  if (stbtt_InitFont(&font, font_buffer.data(), 0) == 0) {
    std::cerr << "Failed to load font" << std::endl;
    return 1;
  }

  // Initialise all font values
  const auto scale = stbtt_ScaleForPixelHeight(&font, font_size);
  int ascent, descent, line_gap;
  stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
  ascent = static_cast<int>(ascent * scale);
  descent = static_cast<int>(descent * scale);
  line_gap = static_cast<int>(line_gap * scale);

  // Stores the bitmaps of all characters
  StringBitmap all_characters;

  // Loop over all characters
  auto current_ascent = ascent;
  float current_x = 0.0f;
  for (size_t i = 0; i < input_string.length(); i++) {
    // If we've found a new line we should advance to the next line
    if (input_string[i] == '\n') {
      current_x = 0.0f;
      current_ascent += ascent - descent + line_gap;
      continue;
    }

    // Get all metrics from the current character
    int advance, lsb, x0, y0, x1, y1;
    const auto x_shift = current_x - floorf(current_x);
    stbtt_GetCodepointHMetrics(&font, input_string[i], &advance, &lsb);
    stbtt_GetCodepointBitmapBoxSubpixel(&font, input_string[i], scale, scale,
                                        x_shift, 0, &x0, &y0, &x1, &y1);

    // Allocate memory for the bitmap
    FontBitmap fb;
    fb.pixel_width = x1 - x0;
    fb.pixel_height = y1 - y0;
    fb.pixels.resize(fb.pixel_width * fb.pixel_height);
    // Create the bitmap
    stbtt_MakeCodepointBitmapSubpixel(&font, fb.pixels.data(), fb.pixel_width,
                                      fb.pixel_height, fb.pixel_width, scale,
                                      scale, x_shift, 0, input_string[i]);

    // Insert the bitmap into all_characters
    all_characters.bitmaps.emplace_back(std::move(fb));
    all_characters.x_positions.emplace_back(
        std::max(static_cast<int>(current_x + x0), 0));
    all_characters.y_positions.emplace_back(current_ascent + y0);

    // Advance the x position
    if (current_x + x0 < 0) {
      current_x -= x0;
    }
    current_x += advance * scale;
    // Add kerning
    if (i != input_string.length() - 1) {
      current_x += scale * stbtt_GetCodepointKernAdvance(&font, input_string[i],
                                                         input_string[i + 1]);
    }
  }

  size_t complete_width = 0;
  size_t complete_height = 0;
  // Get the max width and max height
  for (size_t i = 0; i < all_characters.bitmaps.size(); i++) {
    const auto new_height =
        all_characters.y_positions[i] + all_characters.bitmaps[i].pixel_height;
    const auto new_width =
        all_characters.x_positions[i] + all_characters.bitmaps[i].pixel_width;

    if (new_height > complete_height)
      complete_height = new_height;
    if (new_width > complete_width) {
      complete_width = new_width;
    }
  }
  std::cout << "Complete Width: " << complete_width << " px" << std::endl;
  std::cout << "Complete Height: " << complete_height << " px" << std::endl;

  // Allocate memory for the complete string
  std::vector<unsigned char> complete_string(complete_width * complete_height);
  // Write all bitmaps into the complete string
  for (size_t c = 0; c < all_characters.bitmaps.size(); c++) {
    const auto x = all_characters.x_positions[c];
    const auto y = all_characters.y_positions[c];
    for (size_t i = 0; i < all_characters.bitmaps[c].pixel_width; i++) {
      for (size_t j = 0; j < all_characters.bitmaps[c].pixel_height; j++) {
        // Clip when y gets below 0
        if (static_cast<long>(j) + y < 0) {
          continue;
        }
        const auto complete_index = (i + x) + (j + y) * complete_width;
        const auto bitmap_index = i + j * all_characters.bitmaps[c].pixel_width;

        complete_string[complete_index] =
            all_characters.bitmaps[c].pixels[bitmap_index];
      }
    }
  }

  // Output all pixels into the output file
  std::ofstream output_file(output_file_name);
  if (output_file.fail()) {
    std::cerr << "Failed to open output file" << std::endl;
    return 1;
  }
  constexpr char print_chars[] = " .:ioVM@";
  for (size_t i = 0; i < complete_height; i++) {
    for (size_t j = 0; j < complete_width; j++) {
      output_file << print_chars[complete_string[i * complete_width + j] >> 5];
    }
    output_file << std::endl;
  }
  output_file.close();

  std::cout << "Successfully wrote the Bee Movie Script into "
            << default_output_file_name << std::endl;

  return 0;
}