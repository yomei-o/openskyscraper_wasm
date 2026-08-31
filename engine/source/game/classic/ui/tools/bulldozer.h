#ifndef OSS_CLASSIC_UI_TOOLS_BULLDOZER_H
#define OSS_CLASSIC_UI_TOOLS_BULLDOZER_H

//tools.h before tool.h: tool.h pulls in tools.h, whose trailing
//includes only define Tool if tools.h was entered first.
#include "tools.h"
#include "tool.h"


namespace OSS {
	namespace Classic {
		class Item;

		/**
		 * Demolishes the item under the cursor.
		 *
		 * The tower already knew how to do this - TowerStructure::removeItem has
		 * always been there - it just had nothing asking it to.
		 */
		class BulldozerTool : public Tool {
		public:
			BulldozerTool(ToolsUI * ui) : Tool(ui) {}

			virtual bool eventMouseDown(MouseButtonEvent * event);
			virtual bool eventMouseMove(MouseMoveEvent * event);

		private:
			//Where the cursor last was, in cells, so a click and a move agree.
			int2 cursorCell;

			Item * itemUnderCursor();
		};
	}
}


#endif
