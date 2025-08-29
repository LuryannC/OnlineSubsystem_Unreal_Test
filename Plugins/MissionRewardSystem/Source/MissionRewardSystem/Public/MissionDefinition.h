// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MissionRewardDataTypes.h"
#include "Engine/DataAsset.h"
#include "MissionDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MISSIONREWARDSYSTEM_API UMissionDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mission")
	FMissionStruct MissionData;
};
