#ifndef OSS_CLASSIC_UI_TOOLS_CONSTRUCTION_H
#define OSS_CLASSIC_UI_TOOLS_CONSTRUCTION_H

#include "tool.h"

#include "tools.h"
#include "../../items/itemdescriptor.h"


namespace OSS {
	namespace Classic {
		class ConstructionTool : public Tool {
			
			/**
			 * Construction
			 */
		public:
			ConstructionTool(ToolsUI * ui);
			
			
			/**
			 * Item
			 */
		private:
			ItemType itemType;
			ItemDescriptor * itemDescriptor;
			
		public:
			ItemType getItemType();
			void setItemType(ItemType type);
			
			ItemDescriptor * getItemDescriptor();
			void setItemDescriptor(ItemDescriptor * descriptor);
			
			
			/**
			 * Template
			 */
		private:
			recti initialTemplateRect;
			recti templateRect;
			
			double2 templateCenter;
			
		public:
			double2 getTemplateCenter();
			void setTemplateCenter(double2 center);
			
			
			
			/**
			 * State
			 */
		public:
			virtual void update();
			virtual void updateTemplateRect();
			
			Updatable::Conditional<ConstructionTool> updateTemplateRectIfNeeded;
			
			
			/**
			 * Drawing
			 */
		public:
			virtual void draw(rectd dirtyRect);
			
			
			/**
			 * Event Handling
			 */
		private:
			bool isDraggingConstruction;
			
			//A shaft is dragged out to its height and built once, on release.
			//Unlike a flexible width item it is not built segment by segment as
			//the mouse moves: a shaft has to be validated over its whole span,
			//and a partial one would already be in the way of the rest.
			bool isDraggingShaft;
			recti shaftInitialRect;
			
		public:
			virtual bool eventMouseDown(MouseButtonEvent * event);
			virtual bool eventMouseUp(MouseButtonEvent * event);
			virtual bool eventMouseMove(MouseMoveEvent * event);
		};
	}
}


#endif
