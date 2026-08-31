#ifndef OSS_CLASSIC_UI_TOOLS_INSPECTION_H
#define OSS_CLASSIC_UI_TOOLS_INSPECTION_H

//tools.h before tool.h: tool.h pulls in tools.h, whose trailing
//includes only define Tool if tools.h was entered first.
#include "tools.h"
#include "tool.h"


namespace OSS {
	namespace Classic {
		/**
		 * Reports what is under the cursor in the status bar.
		 *
		 * SimTower opened a window per item kind; this says the same facts in
		 * one line, which is what the message bar is for and needs no new UI.
		 */
		class InspectionTool : public Tool {
		public:
			InspectionTool(ToolsUI * ui) : Tool(ui) {}

			virtual bool eventMouseDown(MouseButtonEvent * event);
		};
	}
}


#endif
