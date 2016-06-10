// Helper functions for a two-dimensional XY matrix of pixels.
// Special credit to Mark Kriegsman
//
// 2014-10-18 - Special version for RGB Shades Kickstarter
//              https://www.kickstarter.com/projects/macetech/rgb-led-shades
//              2014-10-18 - code version 2c (local table, holes are r/w), 
//              by Mark Kriegsman 
//
//              This special 'XY' code lets you program the RGB Shades
//              as a plain 16x5 matrix.  
//
//              Writing to and reading from the 'holes' in the layout is 
//              also allowed; holes retain their data, it's just not displayed.
//
//              You can also test to see if you're on or off the layout
//              like this
//                if( XY(x,y) > LAST_VISIBLE_LED ) { ...off the layout...}
//
//              X and Y bounds checking is also included, so it is safe
//              to just do this without checking x or y in your code:
//                leds[ XY(x,y) ] == CRGB::Red;
//              All out of bounds coordinates map to the first hidden pixel.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;


// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 13;
const uint8_t kBorderWidth = 1;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);
CRGB led_buffer_plus_safety_pixel[NUM_LEDS + 1];
CRGB* const led_buffer(led_buffer_plus_safety_pixel + 1);

#define LAST_VISIBLE_LED 111
uint8_t XY( uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint8_t ShadesTable[] = {
    112, 113, 114, 111, 110, 109, 115, 116, 117, 118, 108, 107, 106, 119, 120, 121,
    122, 123,  96,  97,  98,  99, 100, 124, 125, 101, 102, 103, 104, 105, 126, 127,
    128,  95,  94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82, 129,
     66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,
    130,  65,  64,  63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52, 131,
    135, 134,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 133, 132,
    136, 137, 138,  39,  38,  37,  36,  35,  34,  33,  32,  31,  30, 139, 140, 141,
    149, 148, 147, 146,  22,  23,  24,  25,  26,  27,  28,  29, 145, 144, 143, 142,
    150, 151, 152, 153, 154,  21,  20,  19,  18,  17,  16, 155, 156, 157, 158, 159,
    169, 168, 167, 166, 165,  10,  11,  12,  13,  14,  15, 164, 163, 162, 161, 160,
    170, 171, 172, 173, 174, 175,   9,   8,   7,   6, 176, 177, 178, 179, 180, 181,
    193, 192, 191, 190, 189, 188,   2,   3,   4,   5, 187, 186, 185, 184, 183, 182,
    194, 195, 196, 197, 198, 199, 200,   1,   0, 201, 202, 203, 204, 205, 206, 207
  };

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = ShadesTable[i];
  return j;
}


