// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MissionRewardSettings.generated.h"

class UMissionsAsset;
/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta = (DisplayName="Mission Reward System"))
class MISSIONREWARDSYSTEM_API UMissionRewardSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMissionRewardSettings();
	
	/** List of mission PrimaryDataAssets loaded at startup */
	UPROPERTY(EditAnywhere, config, Category = "Sources")
	TArray<TSoftObjectPtr<UMissionsAsset>> Missions;

	/* If false subscribe to the save delegates in the subsystem and do your own custom logic with the data */
	UPROPERTY(EditAnywhere, config, Category = "Save")
	bool bSaveLocally = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Save")
	FString SaveSlotName;
	
	UPROPERTY(EditAnywhere, config, Category = "Save")
	int32 SaveUserIndex = 0;

	UPROPERTY(EditAnywhere, config, Category = "Debug")
	bool bShowDebugMessages = false;
};
