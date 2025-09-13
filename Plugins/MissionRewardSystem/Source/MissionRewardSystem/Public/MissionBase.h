// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

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

	/** Get the info about this mission, such as ID, Title, Description, etc...*/
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	FMissionStruct GetMissionData() const;

	/** Get the conditions to complete info for this mission.*/
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	const TArray<FMissionCondition> GetMissionConditions() const { return MissionData.Conditions; }

	/** Called whenever this mission conditions progress.*/
	UPROPERTY(BlueprintAssignable)
	FMissionProgressUpdated OnProgressUpdated;

	/** Called when this mission is completed. */
	UPROPERTY(BlueprintAssignable)
	FMissionCompleted OnMissionCompleted;

public:
	// Called by the subsystem.
	UFUNCTION()
	void SetInstanceID(const FGuid& InID) { MissionData.InstanceID = InID; }
	void UpdateRuntimeConditionsProgress(const TArray<FMissionCondition>& InMissionCondition);
	void OnGameplayEvent(const FGameplayTag& EventTag, int32 Amount);
	void SetMissionCompleted();

private:
	UPROPERTY(EditDefaultsOnly, Category="MissionRewardSystem")
	FMissionStruct MissionData;

	void ShowMissionDebugData();
};
