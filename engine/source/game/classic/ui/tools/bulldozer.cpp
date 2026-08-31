#include "bulldozer.h"

#include "../game.h"

using namespace OSS;
using namespace Classic;


bool BulldozerTool::eventMouseMove(MouseMoveEvent * event)
{
	//Tracked on move as well as on click so the cell is already known by the
	//time the button goes down, the way the construction tool does it.
	double2 world = ui->getScene()->windowToWorld(event->position);
	cursorCell = ui->getTower()->structure->worldToCell(rectd(world, double2(1, 1))).origin;
	return false;
}

Item * BulldozerTool::itemUnderCursor()
{
	TowerStructure::ItemSet items =
		ui->getTower()->structure->getItems(recti(cursorCell, int2(1, 1)));
	if (items.empty()) return NULL;
	return *items.begin();
}

bool BulldozerTool::eventMouseDown(MouseButtonEvent * event)
{
	//The cursor position from the last move can be stale if the button went
	//down without a move first, so recompute it here.
	double2 world = ui->getScene()->windowToWorld(event->position);
	cursorCell = ui->getTower()->structure->worldToCell(rectd(world, double2(1, 1))).origin;

	Item * item = itemUnderCursor();
	if (!item) {
		ui->ui->setStatusMessage("NOTHING TO DEMOLISH THERE");
		return true;
	}

	//The lobby and the metro cannot go: the tower would have no ground floor
	//and no way in.
	if (item->descriptor && (item->descriptor->attributes & kUndestructibleAttribute)) {
		ui->ui->setStatusMessage("CANNOT DEMOLISH " +
								 Item::nameForItemType(item->getType()));
		return true;
	}

	string name = Item::nameForItemType(item->getType());
	ui->getTower()->structure->removeItem(item);
	ui->ui->setStatusMessage("DEMOLISHED " + name);

	return true;
}
