#ifndef OSS_GAME_UI_WINDOW_H
#define OSS_GAME_UI_WINDOW_H

#include "view.h"


namespace OSS {
	class Window : public View {
	public:
		Window();
		virtual void draw(rectd dirtyRect);

		/**
		 * Dragging
		 *
		 * A window is moved by grabbing it anywhere a subview does not claim.
		 * No hit testing against the subviews is needed here: sendEvent()
		 * offers an event to the subviews before the view itself, so a button
		 * always wins over a drag.
		 *
		 * A Window is not a Responder, so a subclass that wants to be movable
		 * calls these from its own mouse handlers.  All three take root-view
		 * coordinates, i.e. MouseEvent::position as it arrives.
		 */
	private:
		bool dragging;
		double2 dragOffset;	//grab point, relative to the frame origin

	public:
		bool isDragging() { return dragging; }
		bool beginDrag(double2 position);
		bool continueDrag(double2 position);
		bool endDrag();
	};
}


#endif
