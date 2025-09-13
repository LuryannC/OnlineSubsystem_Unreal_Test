// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MissionRewardSystemLog.h"
#include "RewardBase.generated.h"

class APlayerController;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MISSIONREWARDSYSTEM_API URewardBase : public UObject
{
	GENERATED_BODY()


public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category="Reward")
	bool GrantReward();
	virtual bool GrantReward_Implementation() { return true;}
};
