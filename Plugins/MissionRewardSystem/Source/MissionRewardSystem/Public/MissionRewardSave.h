// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

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
	TArray<FCompletedMission> CompletedMissions;
	// TArray<FName> CompletedMissionIDs;
	
	UPROPERTY()
	TArray<FProgressedMissions> OnGoingMissionsProgress;

	UPROPERTY()
	TArray<FGrantedMission> GrantedMissions;
	// TArray<TSoftClassPtr<UMissionBase>> GrantedMissions;
};
