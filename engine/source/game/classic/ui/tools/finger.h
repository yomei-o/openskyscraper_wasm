#ifndef OSS_CLASSIC_UI_TOOLS_FINGER_H
#define OSS_CLASSIC_UI_TOOLS_FINGER_H

//tools.h before tool.h: tool.h pulls in tools.h, whose trailing
//includes only define Tool if tools.h was entered first.
#include "tools.h"
#include "tool.h"


namespace OSS {
	namespace Classic {
		/**
		 * The neutral pointer.
		 *
		 * It deliberately does nothing with the mouse.  Its value is that while
		 * it is the selected tool, the construction tool is not - so the tower
		 * can be looked at and scrolled without a click putting a floor down.
		 * Until now there was no way to stop building.
		 */
		class FingerTool : public Tool {
		public:
			FingerTool(ToolsUI * ui) : Tool(ui) {}
		};
	}
}


#endif
