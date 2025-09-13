// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#pragma once

#include "CoreMinimal.h"
#include "MissionBase.h"
#include "Engine/DataAsset.h"
#include "Engine/PrimaryAssetLabel.h"
#include "MissionsAsset.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API UMissionsAsset : public UPrimaryAssetLabel
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Asset Settings")
	FPrimaryAssetType DataType;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	TArray<TSoftClassPtr<UMissionBase>> Missions;
};
