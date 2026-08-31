#ifndef OSS_GAME_CLASSIC_TOWER_FUNDS_H
#define OSS_GAME_CLASSIC_TOWER_FUNDS_H

#include "tower.h"


namespace OSS {
	namespace Classic {		
		class TowerFunds : public GameObject {
			
			/**
			 * Cosntruction
			 */
		public:
			const Pointer<Tower> tower;
			
			TowerFunds(Tower * tower);
			
			
			/**
			 * Funds
			 */
		private:
			long funds;
			Pointer<SoundEffect> transferSound;
			
			//Debugging: money still comes in, but nothing costs anything.
			bool unlimited;
			
		public:
			long getFunds();
			void setFunds(long f);
			
			bool isUnlimited();
			void setUnlimited(bool u);
			
			void transfer(long amount);
			bool hasSufficient(long requestedAmount);
		};
	}
}


#endif
