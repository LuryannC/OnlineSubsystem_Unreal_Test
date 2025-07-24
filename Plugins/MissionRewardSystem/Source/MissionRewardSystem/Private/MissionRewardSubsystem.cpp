// Fill out your copyright notice in the Description page of Project Settings.
#include "MissionRewardSubsystem.h"
#include "MissionRewardSave.h"
#include "MissionRewardSettings.h"
#include "MissionRewardSystemLog.h"
#include "RewardBase.h"
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
	CompletedMissionAssets.Empty();
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
			CompletedMissionAssets = Save->CompletedMissionAssets;
			OnGoingMissionsProgress = Save->OnGoingMissionsProgress;
		}
	}

	LoadMissionAssets();
}

void UMissionRewardSubsystem::LoadProgress(UMissionRewardSave* SaveData)
{
	if (!SaveData)
	{
		return;
	}
	
	CompletedMissionIDs = SaveData->CompletedMissionIDs;
	CompletedMissionAssets = SaveData->CompletedMissionAssets;
	OnGoingMissionsProgress = SaveData->OnGoingMissionsProgress;

	OnMissionRewardSystemLoaded.Broadcast(SaveData);
	LoadMissionAssets();
}

void UMissionRewardSubsystem::LoadMissionAssets()
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		TArray<UMissionAsset*> Assets;
		
		for (const TSoftObjectPtr<UMissionAsset>& MissionAsset : Settings->MissionAssets)
		{
			if (UMissionAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
			{
				Assets.Add(CurrentMissionAsset);
			}
		}

		for (const auto& SeasonalMissionAsset : Settings->SeasonalMissionAssets)
		{
			if (!SeasonalMissionAsset.IsValid())
			{
				UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::LoadMissionAssets - Seasonal Mission Asset nullptr"));
				break;
			}
			
			for (const TSoftObjectPtr<UMissionAsset>& MissionAsset: SeasonalMissionAsset->MissionAssets)
			{
				if (UMissionAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
				{
					Assets.Add(CurrentMissionAsset);
				}	
			}
		}
		
		LoadMissionAssets(Assets);
	}
}

void UMissionRewardSubsystem::LoadMissionAssets(const TArray<UMissionAsset*>& Assets)
{
	for (UMissionAsset* Asset : Assets)
	{
		if (!Asset || CompletedMissionIDs.Contains(Asset->MissionID))
		{
			continue;
		}

		InitialiseMission(Asset);
	}
}

void UMissionRewardSubsystem::InitialiseMission(UMissionAsset* Asset)
{
	UMissionBase* Instance = NewObject<UMissionBase>(this);
	Instance->InitialiseMission(Asset);

	Instance->OnProgressUpdated.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionProgressUpdated);
	Instance->OnMissionCompleted.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionCompleted);

	for (const auto& ProgressedMission : OnGoingMissionsProgress)
	{
		if (Asset->MissionID == ProgressedMission.MissionID)
		{
			Instance->SetRuntimeConditionsFromSavedFile(ProgressedMission.ConditionsProgress);
			UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::InitialiseMission - Updated conditions from existing for Mission: %s"), *Asset->MissionID.ToString());
			break;
		}
	}

	ActiveMissions.Add(Instance);
	OnMissionAdded.Broadcast(Instance);
		
	for (const FMissionCondition& Condition : Asset->Conditions)
	{
		ListenerMap.Add(Condition.EventTag, Instance);
	}
	
	UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::InitialiseMission - Mission %s initialised"), *Asset->MissionID.ToString());
}

void UMissionRewardSubsystem::GrantMission(UMissionAsset* InMissionAsset, const bool bAllowDuplicates)
{
	if (!InMissionAsset) return;

	// Checking if this mission should be added again.
	if (CompletedMissionIDs.Contains(InMissionAsset->MissionID) && !bAllowDuplicates)
	{
		return;
	}

	InitialiseMission(InMissionAsset);
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

	CompletedMissionIDs.AddUnique(Mission->GetMissionAssetData()->MissionID);
	CompletedMissionAssets.AddUnique(const_cast<UMissionAsset*>(Mission->GetMissionAssetData()));
	ActiveMissions.Remove(Mission);

	OnMissionCompleted.Broadcast(Mission->GetMissionAssetData()->MissionID.ToString());

	// Remove from ongoing
	const FName MissionID = Mission->GetMissionAssetData()->MissionID;
	FMissionStruct OnGoingMissionStruct({MissionID, Mission->GetRuntimeConditions()});
	int32 IndexToRemove = 0;
	for (int i = 0; i < OnGoingMissionsProgress.Num() - 1; ++i)
	{
		if (OnGoingMissionsProgress[i].MissionID == MissionID)
		{
			IndexToRemove = i;
			
			break;
		}
	}
	OnGoingMissionsProgress.RemoveAt(IndexToRemove);

	UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::HandleMissionCompleted - Mission %s completed!"), *MissionID.ToString());

	GiveMissionRewards(Mission);

	CommitSave();
}

void UMissionRewardSubsystem::HandleMissionProgressUpdated(UMissionBase* Mission)
{
	if (!Mission) return;


	const FName MissionID = Mission->GetMissionAssetData()->MissionID;
	const FMissionStruct NewProgress({MissionID, Mission->GetRuntimeConditions()});

	// Look for existing index
	const int32 ExistingIndex = OnGoingMissionsProgress.IndexOfByPredicate([&](const FMissionStruct& Entry)
	{
		return Entry.MissionID == MissionID;
	});


	if (ExistingIndex != INDEX_NONE)
	{
		// Updating existing entry
		for (int i = 0; i < OnGoingMissionsProgress[ExistingIndex].ConditionsProgress.Num() - 1; ++i)
		{
			const int32 PreviousProgressAmount = OnGoingMissionsProgress[ExistingIndex].ConditionsProgress[i].Current;
			const int32 NewProgressAmount = NewProgress.ConditionsProgress[i].Current;
			FString ConditionName = OnGoingMissionsProgress[ExistingIndex].ConditionsProgress[i].EventTag.ToString();
			
			UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::HandleMissionProgressUpdated - Updated %s progress for %s from %i to %i"), *MissionID.ToString(), *ConditionName, PreviousProgressAmount, NewProgressAmount);
		}
		
		OnGoingMissionsProgress[ExistingIndex] = NewProgress;
	}
	else
	{
		// Adding new entry
		OnGoingMissionsProgress.Add(NewProgress);
		UE_LOG(MissionRewardSystemLog, Display, TEXT("UMissionRewardSubsystem::HandleMissionProgressUpdated - New progress registered for %s!"), *MissionID.ToString());
	}

	if (Mission->bIsCompleted)
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
			Save->CompletedMissionAssets = CompletedMissionAssets; 
			Save->OnGoingMissionsProgress = OnGoingMissionsProgress;

			if (Settings->bSaveLocally)
			{
				UGameplayStatics::SaveGameToSlot(Save, Settings->SaveSlotName, Settings->SaveUserIndex);
			}
			OnMissionRewardSystemSaved.Broadcast(Save);
		}
	}
}

void UMissionRewardSubsystem::GiveMissionRewards(const UMissionBase* Mission) const
{
	APlayerController* PC = UGameplayStatics::GetGameInstance(this)->GetFirstLocalPlayerController();
	if (PC)
	{
		for (const auto& Reward : Mission->GetMissionAssetData()->Rewards)
		{
			if (Reward.Get()->GrantReward_Implementation(PC))
			{
				OnRewardUnlocked.Broadcast(Mission->GetMissionAssetData()->MissionID.ToString(), true, Success);
				UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GiveMissionRewards - Reward was given!"));
			}
			else
			{
				OnRewardUnlocked.Broadcast(Mission->GetMissionAssetData()->MissionID.ToString(), false, Unknown);
				UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GiveMissionRewards - Failed to give reward for mission %s!"), *Mission->GetMissionAssetData()->MissionID.ToString());
			}
		}
	}
}
