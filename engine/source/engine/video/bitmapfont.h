#ifndef OSS_ENGINE_VIDEO_BITMAPFONT_H
#define OSS_ENGINE_VIDEO_BITMAPFONT_H

#include "../external.h"

#include "../types.h"
#include "../math/headers.h"


namespace OSS {
	/**
	 * A small bitmap font compiled into the binary.
	 *
	 * SimTower drew its numbers with a GDI font, so there is no glyph art in
	 * its resources to reuse, and the engine has no text rendering of its own.
	 * Rather than pull in a font library for a handful of digits, the glyphs are
	 * a table here and are drawn as solid quads - one per horizontal run of set
	 * pixels, so a five-digit number costs a few dozen quads.
	 *
	 * Only the characters needed to print an amount of money and a count are
	 * defined; anything else advances as a space.  Coordinates are the engine's,
	 * i.e. y grows upwards, and the origin is the bottom left of the text.
	 */
	class BitmapFont {
	public:
		static const int kGlyphHeight = 7;

		static double width(const std::string & text, double scale = 1);
		static void draw(const std::string & text, double2 origin,
						 color4d color, double scale = 1);

		//Right-aligned at x, which is where the text ends rather than starts.
		static void drawRightAligned(const std::string & text, double2 end,
									 color4d color, double scale = 1);

		//"$20,000,000" and "1,234" - the grouping is what makes a seven-figure
		//number readable at a glance, which is the whole point of the display.
		static std::string formatMoney(long amount);
		static std::string formatNumber(long value);
	};
}


#endif
