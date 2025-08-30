// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MissionRewardDataTypes.h"
#include "MissionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionProgressUpdated, UMissionBase*, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionCompleted, UMissionBase*, Mission);

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MISSIONREWARDSYSTEM_API UMissionBase : public UObject
{
	GENERATED_BODY()

public:
	void InitialiseMission();

	UFUNCTION()
	FMissionStruct GetMissionData() const;

	UFUNCTION(BlueprintCallable)
	FMissionStruct BP_GetMissionData() const;
	
	UFUNCTION(BlueprintCallable)
	void SetMissionCompleted();

	UFUNCTION(BlueprintCallable)
	TArray<FMissionCondition> GetMissionConditions() { return MissionData.Conditions; }

	void UpdateRuntimeConditionsProgress(const TArray<FMissionCondition>& InMissionCondition);

	/** Called by the subsystem whenever a gameplay event occurs */
	void OnGameplayEvent(const FGameplayTag& EventTag, int32 Amount);
	
	UPROPERTY(BlueprintAssignable)
	FMissionProgressUpdated OnProgressUpdated;

	UPROPERTY(BlueprintAssignable)
	FMissionCompleted OnMissionCompleted;

private:
	UPROPERTY(EditDefaultsOnly)
	FMissionStruct MissionData;

	void ShowMissionDebugData();
};
