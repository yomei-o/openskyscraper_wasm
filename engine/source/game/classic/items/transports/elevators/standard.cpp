#include "standard.h"

using namespace OSS;
using namespace Classic;


ItemDescriptor StandardElevatorItem::descriptor = {
	kStandardElevatorType,
	kElevatorGroup,
	kTransportCategory,
	1,
	(kFlexibleHeightAttribute),
	200000,
	int2(4, 1),
	int2(4, 1),		//minUnit: one shaft segment
	rectmaski(),
	30				//SimTower caps a shaft at 30 floors
};





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

StandardElevatorItem::StandardElevatorItem(Tower * tower) : ElevatorItem(tower, &descriptor)
{
}
