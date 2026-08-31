#include "funds.h"

#include "tower.h"

using namespace OSS;
using namespace Classic;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Construction
//----------------------------------------------------------------------------------------------------

TowerFunds::TowerFunds(Tower * tower) : tower(tower), unlimited(false)
{
	//Set the initial money to 2'000'000
	//TODO: change back to 2e6
	funds = 20e6;
	
	//Initialize the funds transfer sound effect.
	transferSound = new SoundEffect();
	transferSound->sound = Sound::named("simtower/cash");
	transferSound->layer = SoundEffect::kTopLayer;
	transferSound->minIntervalBetweenPlaybacks = 0.1;
	transferSound->loopCount = 2;
	transferSound->copyBeforeUse = true;
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Funds
//----------------------------------------------------------------------------------------------------

long TowerFunds::getFunds()
{
	return funds;
}

void TowerFunds::setFunds(long f)
{
	if (funds != f) {
		funds = f;
		
		//Play the funds transfer sound effect. Sounds like an old cashier.
		Audio::shared()->play(transferSound);
	}
}

bool TowerFunds::isUnlimited()
{
	return unlimited;
}

void TowerFunds::setUnlimited(bool u)
{
	unlimited = u;
}

void TowerFunds::transfer(long amount)
{
	//OSSObjectLog << amount << "€" << std::endl;
	
	//Spending is free while the funds are unlimited.  Income is left alone
	//rather than also suppressed, so the number still shows the tower
	//earning and only ever moves in one direction.
	if (unlimited && amount < 0)
		return;
	
	setFunds(getFunds() + amount);
}

bool TowerFunds::hasSufficient(long requestedAmount)
{
	if (unlimited) return true;
	return (getFunds() >= requestedAmount);
}
