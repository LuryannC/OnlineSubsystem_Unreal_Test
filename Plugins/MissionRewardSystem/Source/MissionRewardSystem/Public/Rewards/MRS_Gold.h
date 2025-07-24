// Fill out your copyright notice in the Description page of Project Settings.

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

	virtual bool GrantReward_Implementation(APlayerController* Recipient) override;
};
