#ifndef OSS_CLASSIC_UI_CONTROL_WINDOW_H
#define OSS_CLASSIC_UI_CONTROL_WINDOW_H

#include "../game.h"

#include "../../tower/tower.h"


namespace OSS {
	namespace Classic {
		class WatchView;
		class RatingView;
		
		class ControlWindow : public Window {
			
			/**
			 * Construction
			 */
		public:
			const Pointer<GameUI> ui;
			
			ControlWindow(GameUI * ui);
			
			//TODO: move this somewhere else
			Tower * getTower();
			
			
			/**
			 * Subviews
			 */
		private:
			Pointer<WatchView> watchView;
			Pointer<RatingView> ratingView;
			
			
			/**
			 * Status Message
			 *
			 * The long sunken field in the middle of the background is
			 * SimTower's message bar, which the original filled with GDI text.
			 * A posted message shows for a while and then gives way to the date,
			 * so the bar is never just empty.
			 */
		private:
			string message;
			double messageExpiry;		//tower time at which it stops showing
			int lastRating;				//-1 until the first update
			unsigned int lastYear;

		public:
			void setMessage(string text);
			string getDisplayedMessage();
			virtual void update();


			/**
			 * Drawing
			 */
		private:
			Pointer<Texture> backgroundTexture;
			
		public:
			virtual void draw(rectd dirtyRect);
		};
	}
}


#include "watch.h"
#include "rating.h"


#endif
