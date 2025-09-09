// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MissionRewardDataTypes.h"
#include "GameFramework/SaveGame.h"
#include "MissionsAsset.h"
#include "MissionRewardSave.generated.h"

class UMissionBase;
/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API UMissionRewardSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FName> CompletedMissionIDs;
	
	UPROPERTY()
	TArray<FProgressedMissions> OnGoingMissionsProgress;
};
