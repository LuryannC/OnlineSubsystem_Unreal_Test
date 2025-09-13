// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.


#include "Rewards/MRS_Gold.h"

bool UMRS_Gold::GrantReward_Implementation()
{
	UE_LOG(MissionRewardSystemLog, Display, TEXT("UMRS_Gold::GrantReward_Implementation - rewarded with %i gold(s)"), Amount);

	return true;
}
