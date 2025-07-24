// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MissionRewardSystemLog.h"
#include "RewardBase.generated.h"

class APlayerController;
/**
 * 
 */
UCLASS(Abstract, Blueprintable,EditInlineNew, DefaultToInstanced)
class MISSIONREWARDSYSTEM_API URewardBase : public UObject
{
	GENERATED_BODY()


public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category="Reward")
	bool GrantReward(APlayerController* Recipient);
	virtual bool GrantReward_Implementation(APlayerController* Recipient) { return true;}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> RewardIcon;
};
