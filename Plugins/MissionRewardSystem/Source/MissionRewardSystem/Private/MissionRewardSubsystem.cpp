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
	AvailableMissions.Empty();
	
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

	LoadMissionAssets();
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
	LoadMissionAssets();
}

const TArray<UMissionBase*>& UMissionRewardSubsystem::GetActiveMissions() const
{
	return ActiveMissions;
}

void UMissionRewardSubsystem::LoadMissionAssets()
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		TArray<UMissionAsset*> Assets;

		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::LoadMissionAssets - Available Mission Assets: %i"), Settings->MissionAssets.Num());
		for (const TSoftObjectPtr<UMissionAsset>& MissionAsset : Settings->MissionAssets)
		{
			if (UMissionAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
			{
				Assets.Add(CurrentMissionAsset);
				AvailableMissions.Add(CurrentMissionAsset->MissionData);
			}
		}

		for (const TSoftObjectPtr<USeasonalMissionAsset>& SeasonalMissionAsset : Settings->SeasonalMissionAssets)
		{
			USeasonalMissionAsset* LoadedSeasonalAsset = SeasonalMissionAsset.LoadSynchronous();
			
			if (!LoadedSeasonalAsset)
			{
				UE_LOG(MissionRewardSystemLog, Error, TEXT("UMissionRewardSubsystem::LoadMissionAssets - Seasonal Mission Asset nullptr"));
				continue;
			}

			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::LoadMissionAssets - Available Seasonal Mission Assets: %i"), Settings->SeasonalMissionAssets.Num());
			for (const TSoftObjectPtr<UMissionAsset>& MissionAsset: LoadedSeasonalAsset->MissionAssets)
			{
				if (UMissionAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
				{
					Assets.Add(CurrentMissionAsset);
					AvailableMissions.Add(CurrentMissionAsset->MissionData);
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
		if (!Asset || CompletedMissionIDs.Contains(Asset->MissionData.MissionID))
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
		if (Asset->MissionData.MissionID == ProgressedMission.MissionID)
		{
			Instance->SetRuntimeConditionsFromSavedFile(ProgressedMission.ConditionsProgress);
			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::InitialiseMission - Updated conditions from existing for Mission: %s"), *Asset->MissionData.MissionID.ToString());
			break;
		}
	}

	ActiveMissions.Add(Instance);
	OnMissionAdded.Broadcast(Instance);
		
	for (const FMissionCondition& Condition : Asset->MissionData.Conditions)
	{
		ListenerMap.Add(Condition.EventTag, Instance);
	}
	
	UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::InitialiseMission - Mission %s initialised"), *Asset->MissionData.MissionID.ToString());
}

void UMissionRewardSubsystem::GrantMission(UMissionAsset* InMissionAsset, const bool bAllowDuplicates)
{
	if (!InMissionAsset) return;

	// Checking if this mission should be added again.
	if (CompletedMissionIDs.Contains(InMissionAsset->MissionData.MissionID) && !bAllowDuplicates)
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

	CompletedMissionIDs.AddUnique(Mission->GetMissionAssetData()->MissionData.MissionID);
	ActiveMissions.Remove(Mission);

	OnMissionCompleted.Broadcast(Mission->GetMissionAssetData()->MissionData.MissionID.ToString());

	const FName MissionID = Mission->GetMissionAssetData()->MissionData.MissionID;
	const int32 IndexToRemove = OnGoingMissionsProgress.IndexOfByPredicate([&](const FMissionStruct& CurrentMission)
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

	CommitSave();
	
	// for (int i = 0; i < OnGoingMissionsProgress.Num() - 1; ++i)
	// {
	// 	if (OnGoingMissionsProgress[i].MissionID == MissionID)
	// 	{
	// 		IndexToRemove = i;
	// 		
	// 		break;
	// 	}
	// }

}

void UMissionRewardSubsystem::HandleMissionProgressUpdated(UMissionBase* Mission)
{
	if (!Mission) return;


	const FName MissionID = Mission->GetMissionAssetData()->MissionData.MissionID;
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
		// for (const auto& Reward : Mission->GetMissionAssetData()->MissionData.Rewards)
		// {
		// 	if (Reward.Get()->GrantReward_Implementation(PC))
		// 	{
		// 		OnRewardUnlocked.Broadcast(Mission->GetMissionAssetData()->MissionData.MissionID.ToString(), true, Success);
		// 		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GiveMissionRewards - Reward was given!"));
		// 	}
		// 	else
		// 	{
		// 		OnRewardUnlocked.Broadcast(Mission->GetMissionAssetData()->MissionData.MissionID.ToString(), false, Unknown);
		// 		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GiveMissionRewards - Failed to give reward for mission %s!"), *Mission->GetMissionAssetData()->MissionData.MissionID.ToString());
		// 	}
		// }
	}
}

FMissionStruct UMissionRewardSubsystem::GetMissionData(const FName MissionId) const
{
	for (const auto& MissionData : AvailableMissions)
	{
		if (MissionData.MissionID == MissionId)
		{
			return MissionData;
		}
	}
	return FMissionStruct{};
}