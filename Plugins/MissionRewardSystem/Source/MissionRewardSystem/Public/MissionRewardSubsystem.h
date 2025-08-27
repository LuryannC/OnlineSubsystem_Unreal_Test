// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissionBase.h"
#include "MissionsAsset.h"
#include "MissionRewardDataTypes.h"
#include "MissionRewardSave.h"
#include "Runtime/Engine/Public/Subsystems/GameInstanceSubsystem.h"
#include "MissionRewardSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionAdded, const UMissionBase*, MissionInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FString&, CompletedMissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRewardUnlocked, const FString&, CompletedMissionID, bool, bWasSuccessful, EUnlockReasonFailReason, FailReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionRewardSystemSaved, UMissionRewardSave*, SaveData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionRewardSystemLoaded, UMissionRewardSave*, SaveData);


/**
 * 
 */
UCLASS()
class MISSIONREWARDSYSTEM_API UMissionRewardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/* Method to give mission at runtime */
	UFUNCTION()
	void GrantMission(UMissionBase* InMission, bool bAllowDuplicates = true);

	/* Method to call whenever a gameplay event that has mission bound to it happen*/
	UFUNCTION(BlueprintCallable)
	void ReportGameplayEvent(const FGameplayTag& EventTag, int32 Amount = 1);

	/* Load assets set in the plugin settings, such as:
	 * Individual missions data asset and Seasonal missions data asset. 
	 */
	void PreInitMissions(const TArray<UMissionsAsset*>& Assets);

	/* In case developer wants to override the values, like retrieving from a cloud save. */
	UFUNCTION(BlueprintCallable)
	void LoadProgress(UMissionRewardSave* SaveData);

	/* Get all active missions */
	UFUNCTION(BlueprintCallable)
	const TArray<UMissionBase*>& GetActiveMissions() const;

	// UFUNCTION(BlueprintCallable)
	// TArray<FMissionStruct> GetAllAvailableMissionsData() const { return AvailableMissions; }
	//
	UFUNCTION(BlueprintCallable)
	FMissionStruct GetMissionData(const FName MissionId) const;

	/* Get the mission IDs (FName) of every completed mission */
	UFUNCTION(BlueprintCallable)
	const TArray<FName>& GetCompletedMissionIDs() const { return CompletedMissionIDs; }

	/* Get the data for every completed mission */
	UFUNCTION(BlueprintCallable)
	const TArray<FMissionStruct>& GetCompletedMissionData() const { return CompletedMissionData; }

	/* Called whenever a mission is added to the player */
	FOnMissionAdded OnMissionAdded;

	/* Called upon completing a mission */
	FOnMissionCompleted OnMissionCompleted;

	/* Called when player receive the mission reward
	 * It is also triggered in case it fails to give the reward */
	FOnRewardUnlocked OnRewardUnlocked;

	/* Called when mission save data file is loaded */
	FOnMissionRewardSystemLoaded OnMissionRewardSystemLoaded;

	/* Called when mission save data is saved */
	FOnMissionRewardSystemSaved OnMissionRewardSystemSaved;
	
protected:
	
	UFUNCTION()
	void HandleMissionCompleted(UMissionBase* Mission);

	UFUNCTION()
	void HandleMissionProgressUpdated(UMissionBase* Mission);

private:	
	void LoadMissions();
	void InitMission(UMissionBase* InMission);
	
	TMultiMap<FGameplayTag, UMissionBase*> ListenerMap;	
	TArray<TObjectPtr<UMissionBase>> ActiveMissions = TArray<TObjectPtr<UMissionBase>>();

	void LoadProgress();
	void CommitSave() const;

	void GiveMissionRewards(UMissionBase* Mission);
	
	UPROPERTY()
	TArray<FName> CompletedMissionIDs;
	
	UPROPERTY()
	TArray<FMissionStruct> OnGoingMissionsProgress;

	UPROPERTY()
	TArray<UMissionBase*> CompletedMissions;
	
	UPROPERTY()
	TArray<FMissionStruct> CompletedMissionData;
};
