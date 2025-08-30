// Fill out your copyright notice in the Description page of Project Settings.
#include "MissionRewardSubsystem.h"
#include "MissionRewardSave.h"
#include "MissionRewardSettings.h"
#include "MissionRewardSystemLog.h"
#include "RewardBase.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

void UMissionRewardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (!Settings->bSaveLocally) return;

		LoadProgress();
	}
}

void UMissionRewardSubsystem::Deinitialize()
{
	ActiveMissions.Empty();
	ListenerMap.Empty();
	CompletedMissionIDs.Empty();
	OnGoingMissionsProgress.Empty();
	
	Super::Deinitialize();
}

void UMissionRewardSubsystem::LoadProgress()
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (const UMissionRewardSave* Save = Cast<UMissionRewardSave>(UGameplayStatics::LoadGameFromSlot(Settings->SaveSlotName, Settings->SaveUserIndex)))
		{
			CompletedMissionIDs = Save->CompletedMissionIDs;
			OnGoingMissionsProgress = Save->OnGoingMissionsProgress;
		}
	}

	LoadMissions();
}

void UMissionRewardSubsystem::LoadProgress(UMissionRewardSave* SaveData)
{
	if (!SaveData)
	{
		return;
	}
	
	CompletedMissionIDs = SaveData->CompletedMissionIDs;
	OnGoingMissionsProgress = SaveData->OnGoingMissionsProgress;

	OnMissionRewardSystemLoaded.Broadcast(SaveData);
	LoadMissions();
}

const TArray<UMissionBase*>& UMissionRewardSubsystem::GetActiveMissions() const
{
	return ActiveMissions;
}

void UMissionRewardSubsystem::LoadMissions()
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		TArray<UMissionsAsset*> Assets;

		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::LoadMissionAssets - Available Mission Assets: %i"), Settings->Missions.Num());
		for (const TSoftObjectPtr<UMissionsAsset>& MissionAsset : Settings->Missions)
		{
			if (UMissionsAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
			{
				Assets.Add(CurrentMissionAsset);
			}
		}		
		PreInitMissions(Assets);
	}
}

void UMissionRewardSubsystem::PreInitMissions(const TArray<UMissionsAsset*>& Assets)
{
	for (UMissionsAsset* Asset : Assets)
	{
		for (auto MissionClass : Asset->Missions)
		{
			if (const UClass* LoadedClass = MissionClass.LoadSynchronous())
			{
				UMissionBase* DefaultMissionData = Cast<UMissionBase>(LoadedClass->GetDefaultObject());
				if (!DefaultMissionData)
				{
					continue;
				}

				const FName MissionID =	DefaultMissionData->GetMissionData().MissionID;

				if (CompletedMissionIDs.Contains(MissionID))
				{
					DefaultMissionData->SetMissionCompleted();
					CompletedMissionData.Add(DefaultMissionData->GetMissionData());
				}
				else
				{
					if (UMissionBase* Mission = NewObject<UMissionBase>(this, LoadedClass))
					{
						InitMission(Mission);
					}
				}
			}
		}
	}
}

void UMissionRewardSubsystem::InitMission(UMissionBase* InMission)
{
	InMission->InitialiseMission();
	
	InMission->OnProgressUpdated.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionProgressUpdated);
	InMission->OnMissionCompleted.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionCompleted);

	for (const auto& ProgressedMission : OnGoingMissionsProgress)
	{
		if (InMission->GetMissionData().MissionID == ProgressedMission.MissionID)
		{
			InMission->UpdateRuntimeConditionsProgress(ProgressedMission.ConditionsProgress);
			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::InitialiseMission - Updated conditions from existing for Mission: %s"), *InMission->GetMissionData().MissionID.ToString());
		}
	}

	ActiveMissions.Add(InMission);
	OnMissionAdded.Broadcast(InMission);
		
	for (const FMissionCondition& Condition : InMission->GetMissionData().Conditions)
	{
		ListenerMap.Add(Condition.EventTag, InMission);
	}
	
	UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::InitialiseMission - Mission %s initialised"), *InMission->GetMissionData().MissionID.ToString());
}

void UMissionRewardSubsystem::GrantMission(UMissionBase* InMission, const bool bAllowDuplicates)
{
	if (!InMission) return;

	// Checking if this mission should be added again.
	if (CompletedMissionIDs.Contains(InMission->GetMissionData().MissionID) && !bAllowDuplicates)
	{
		return;
	}

	InitMission(InMission);
}

void UMissionRewardSubsystem::ReportGameplayEvent(const FGameplayTag& EventTag, int32 Amount)
{
	if (Amount <= 0) { return; }
	
	TArray<UMissionBase*> Listeners;
	ListenerMap.MultiFind(EventTag, Listeners);

	for (UMissionBase* Instance : Listeners)
	{
		Instance->OnGameplayEvent(EventTag, Amount);
	}
}

void UMissionRewardSubsystem::HandleMissionCompleted(UMissionBase* Mission)
{
	if (!Mission) return;

	CompletedMissionIDs.AddUnique(Mission->GetMissionData().MissionID);
	CompletedMissionData.Add(Mission->GetMissionData());
	ActiveMissions.Remove(Mission);

	OnMissionCompleted.Broadcast(Mission->GetMissionData().MissionID.ToString());

	const FName MissionID = Mission->GetMissionData().MissionID;
	const int32 IndexToRemove = OnGoingMissionsProgress.IndexOfByPredicate([&](const FProgressedMissions& CurrentMission)
	{
		return CurrentMission.MissionID == MissionID;
	});
	
	if (IndexToRemove != INDEX_NONE)
	{
		OnGoingMissionsProgress.RemoveAt(IndexToRemove);
	}
	else
	{
		UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionRewardSubsystem::HandleMissionCompleted - Couldn't find Mission index to remove from OngoingMissions."));
	}

	UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::HandleMissionCompleted - Mission %s completed!"), *MissionID.ToString());

	GiveMissionRewards(Mission);

	Mission->OnProgressUpdated.RemoveAll(this);
	Mission->OnMissionCompleted.RemoveAll(this);

	CommitSave();
}

void UMissionRewardSubsystem::HandleMissionProgressUpdated(UMissionBase* Mission)
{
	if (!Mission) return;


	const FName MissionID = Mission->GetMissionData().MissionID;
	const FMissionStruct NewProgress(Mission->GetMissionData());

	// Look for existing index
	const int32 ExistingIndex = OnGoingMissionsProgress.IndexOfByPredicate([&](const FProgressedMissions& Entry)
	{
		return Entry.MissionID == MissionID;
	});


	if (ExistingIndex != INDEX_NONE)
	{
		// Updating existing entry
		for (int i = 0; i < OnGoingMissionsProgress[ExistingIndex].ConditionsProgress.Num() - 1; ++i)
		{
			const int32 PreviousProgressAmount = OnGoingMissionsProgress[ExistingIndex].ConditionsProgress[i].Current;
			const int32 NewProgressAmount = NewProgress.Conditions[i].Current;
			FString ConditionName = OnGoingMissionsProgress[ExistingIndex].ConditionsProgress[i].EventTag.ToString();
			
			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::HandleMissionProgressUpdated - Updated %s progress for %s from %i to %i"), *MissionID.ToString(), *ConditionName, PreviousProgressAmount, NewProgressAmount);
		}
		
		OnGoingMissionsProgress[ExistingIndex] = NewProgress;
	}
	else
	{
		// Adding new entry
		OnGoingMissionsProgress.Add(NewProgress);
		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::HandleMissionProgressUpdated - New progress registered for %s!"), *MissionID.ToString());
	}

	if (Mission->GetMissionData().bIsCompleted)
	{
		HandleMissionCompleted(Mission);
		return;
	}

	CommitSave();
}

void UMissionRewardSubsystem::CommitSave() const
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (UMissionRewardSave* Save = Cast<UMissionRewardSave>(UGameplayStatics::CreateSaveGameObject(UMissionRewardSave::StaticClass())))
		{
			Save->CompletedMissionIDs = CompletedMissionIDs;
			Save->OnGoingMissionsProgress = OnGoingMissionsProgress;

			if (Settings->bSaveLocally)
			{
				UGameplayStatics::SaveGameToSlot(Save, Settings->SaveSlotName, Settings->SaveUserIndex);
			}
			OnMissionRewardSystemSaved.Broadcast(Save);
		}
	}
}

void UMissionRewardSubsystem::GiveMissionRewards(const UMissionBase* Mission)
{
	for (const auto& RewardClass : Mission->GetMissionData().Rewards)
	{
		if (const UClass* LoadedClass = RewardClass.LoadSynchronous())
		{
			URewardBase* RewardInstance = NewObject<URewardBase>(this, LoadedClass);
			if (!RewardInstance)
			{
				continue;
			}

			const bool bWasSuccessful = RewardInstance->GrantReward();
			const FName MissionID = Mission->GetMissionData().MissionID;

			OnRewardUnlocked.Broadcast(MissionID.ToString(), bWasSuccessful, bWasSuccessful ? EUnlockReasonFailReason::Success : EUnlockReasonFailReason::Unknown);
		}
	}
}