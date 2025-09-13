// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#pragma once

#include "CoreMinimal.h"
#include "RewardBase.h"
#include "MRS_Gold.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API UMRS_Gold : public URewardBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	int32 Amount = 100;

	virtual bool GrantReward_Implementation() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	TObjectPtr<UTexture2D> RewardIcon;
};
