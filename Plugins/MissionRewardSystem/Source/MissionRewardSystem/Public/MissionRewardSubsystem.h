// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardReceived, const FString&, CompletedMissionID);

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

	/** Grants a new mission.
	 * @param MissionClass The mission class that will be granted.
	 * @param bAllowDuplicates Should this mission be added if a similar exits? 
	 */	
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	void GrantMission(TSoftClassPtr<UMissionBase> MissionClass, bool bAllowDuplicates = true);

	/** Method to call whenever a gameplay event happens.
	 * @param EventTag The reported event tag.
	 * @param Amount How many it should be added to the progress count.
	 */
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	void ReportGameplayEvent(const FGameplayTag& EventTag, const int32 Amount = 1);

	/** Manually save progress, in case 'bSaveLocally' is false in the plugin settings.
	 * @param SaveData The save where the values should be saved to.
	 */
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	void LoadProgress(UMissionRewardSave* SaveData);

	/** Get all current active missions. */
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	const TArray<UMissionBase*>& GetActiveMissions() const;

	/** Get all completed missions, which include its data. */
	UFUNCTION(BlueprintCallable, Category="MissionRewardSystem")
	const TArray<FCompletedMission>& GetCompletedMissions() const { return CompletedMissions; }

	/** Called whenever a mission is added to the player. */
	FOnMissionAdded OnMissionAdded;

	/** Called upon completing a mission. */
	FOnMissionCompleted OnMissionCompleted;

	/** Called when the player receives the mission reward. */
	FOnRewardReceived OnRewardReceived;

	/** Called when mission save data file is successfully loaded. */
	FOnMissionRewardSystemLoaded OnMissionRewardSystemLoaded;

	/** Called when mission data is successfully saved.
	 * If 'bSaveLocally' is false in the plugin settings,
	 * use the broadcast save data to save it where it must be saved.
	 */
	FOnMissionRewardSystemSaved OnMissionRewardSystemSaved;
	
private:
	
	UFUNCTION()
	void HandleMissionCompleted(UMissionBase* Mission);

	UFUNCTION()
	void HandleMissionProgressUpdated(UMissionBase* Mission);

	void LoadMissions();
	void PreInitMissionsFromAssets(const TArray<UMissionsAsset*>& Assets);
	void InitMission(UMissionBase* InMission);
	void LoadProgress();
	void CommitSave() const;
	void GiveMissionRewards(const UMissionBase* Mission);
	void LoadRuntimeGrantedMissions();

	TMultiMap<FGameplayTag, UMissionBase*> ListenerMap;

	UPROPERTY()
	TArray<TObjectPtr<UMissionBase>> ActiveMissions;

	UPROPERTY()
	TArray<FGrantedMission> GrantedMissions;

	UPROPERTY()
	TArray<FCompletedMission> CompletedMissions;
	
	UPROPERTY()
	TArray<FProgressedMissions> OnGoingMissionsProgress;
};
