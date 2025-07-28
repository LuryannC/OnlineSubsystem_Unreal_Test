// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MissionAsset.h"
#include "Engine/DataAsset.h"
#include "Engine/PrimaryAssetLabel.h"
#include "SeasonalMissionAsset.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API USeasonalMissionAsset : public UPrimaryDataAsset 
{
	GENERATED_BODY()

public:

	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Asset Settings")
	// FPrimaryAssetType DataType;
	//
	// virtual FPrimaryAssetId GetPrimaryAssetId() const override
	// {
	// 	return FPrimaryAssetId(DataType, GetFName());
	// }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Asset Settings")
	FString SeasonID;
	
	UPROPERTY(EditAnywhere, Category = "Sources")
	TArray<TSoftObjectPtr<UMissionAsset>> MissionAssets;	
};
