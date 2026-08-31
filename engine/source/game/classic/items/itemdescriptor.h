#ifndef OSS_GAME_CLASSIC_ITEMS_ITEMDESCRIPTOR_H
#define OSS_GAME_CLASSIC_ITEMS_ITEMDESCRIPTOR_H

#include "../../external.h"


namespace OSS {
	namespace Classic {
		
		//Item Types
		typedef enum {
			kNoType = 0,
			
			//Structure
			kLobbyType,
			kFloorType,
			kStairsType,
			kEscalatorType,
			
			//Elevator
			kStandardElevatorType,
			kServiceElevatorType,
			kExpressElevatorType,
			
			//Office
			kOfficeType,
			
			//Hotel
			kSingleRoomType,
			kDoubleRoomType,
			kSuiteType,
			
			//Entertainment
			kFastFoodType,
			kRestaurantType,
			kShopType,
			kCinemaType,
			kPartyHallType,
			
			//Infrastructure
			kParkingRampType,
			kParkingSpaceType,
			kRecyclingCenterType,
			kMetroType,
			
			//Services
			kCathedrakType,
			kSecurityType,
			kMedicalCenterType,
			kHousekeepingType,
			
			//Condo
			kCondoType,
			
			kMaxType
		} ItemType;
		
		//Item Groups
		typedef enum {
			kNoGroup = 0,
			
			kStructureGroup,
			kElevatorGroup,
			kOfficeGroup,
			kHotelGroup,
			kCondoGroup,
			kEntertainmentGroup,
			kInfrastructureGroup,
			kServicesGroup,
			
			kMaxGroup
		} ItemGroup;
		
		//Item Categories
		typedef enum {
			kFacilityCategory,
			kTransportCategory
		} ItemCategory;
		
		//Item Attributes
		typedef enum {
			kFlexibleWidthAttribute		= (1 << 0),
			kEvery15thFloorAttribute	= (1 << 1),
			kNotAboveGroundAttribute	= (1 << 2),
			kNotBelowGroundAttribute	= (1 << 3),
			kAllowedOnGroundAttribute	= (1 << 4),
			kUndestructibleAttribute	= (1 << 5),
			kFlexibleHeightAttribute	= (1 << 6)	//dragged vertically, like a shaft
		} ItemAttributes;
		
		//Item Descriptor
		typedef struct {
			ItemType type;
			ItemGroup group;
			ItemCategory category;
			unsigned short minRating;
			unsigned short attributes;
			unsigned int price;
			int2 cells;
			int2 minUnit;
			rectmaski mask;
			
			//How far a flexible item may be dragged, in cells.  Zero means no
			//limit, so every existing descriptor keeps its behaviour by leaving
			//the field off the end of its initialiser.
			unsigned short maxSpan;
		} ItemDescriptor;
	}
}


#endif
