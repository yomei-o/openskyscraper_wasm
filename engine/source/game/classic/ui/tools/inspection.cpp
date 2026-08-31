#include "inspection.h"

#include "../game.h"

using namespace OSS;
using namespace Classic;


bool InspectionTool::eventMouseDown(MouseButtonEvent * event)
{
	double2 world = ui->getScene()->windowToWorld(event->position);
	int2 cell = ui->getTower()->structure->worldToCell(rectd(world, double2(1, 1))).origin;

	TowerStructure::ItemSet items =
		ui->getTower()->structure->getItems(recti(cell, int2(1, 1)));

	if (items.empty()) {
		//Still worth answering: it tells you the tool is working and where you
		//actually clicked.
		ui->ui->setStatusMessage("EMPTY  FLOOR " + BitmapFont::formatNumber(cell.y));
		return true;
	}

	Item * item = *items.begin();
	string line = Item::nameForItemType(item->getType());
	line += "   FLOOR " + BitmapFont::formatNumber(item->getRect().minY());
	if (item->descriptor)
		line += "   " + BitmapFont::formatMoney(item->descriptor->price);

	ui->ui->setStatusMessage(line);
	return true;
}
