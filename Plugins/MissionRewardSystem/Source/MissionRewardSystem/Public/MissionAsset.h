// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MissionRewardDataTypes.h"
#include "Engine/DataAsset.h"
#include "RewardBase.h"
#include "MissionAsset.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API UMissionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MissionID = FName();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon = TSoftObjectPtr<UTexture2D>();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMissionCondition> Conditions = TArray<FMissionCondition>();

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Rewards")
    TArray<TObjectPtr<URewardBase>> Rewards = TArray<TObjectPtr<URewardBase>>();
};
