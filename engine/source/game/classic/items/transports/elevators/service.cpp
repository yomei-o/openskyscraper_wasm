#include "service.h"

using namespace OSS;
using namespace Classic;


ItemDescriptor ServiceElevatorItem::descriptor = {
	kServiceElevatorType,
	kElevatorGroup,
	kTransportCategory,
	1,
	(kFlexibleHeightAttribute),
	100000,
	int2(4, 1),
	int2(4, 1),		//minUnit: one shaft segment
	rectmaski(),
	30				//SimTower caps a shaft at 30 floors
};





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

ServiceElevatorItem::ServiceElevatorItem(Tower * tower) : ElevatorItem(tower, &descriptor)
{
}
