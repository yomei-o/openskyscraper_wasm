#ifndef OSS_GAME_DEBUGOPTIONS_H
#define OSS_GAME_DEBUGOPTIONS_H

namespace OSS {
	/**
	 * Switches for looking at the game rather than playing it.
	 *
	 * Set from the command line before the application starts, and read by the
	 * tower as it builds its subsystems.  Nothing here changes behaviour unless
	 * it was asked for on the command line.
	 */
	struct DebugOptions {
		bool unlimitedFunds;	//spending never costs anything
		bool maxRating;			//start at the TOWER rating

		DebugOptions() : unlimitedFunds(false), maxRating(false) {}

		static DebugOptions & shared();
	};
}


#endif
