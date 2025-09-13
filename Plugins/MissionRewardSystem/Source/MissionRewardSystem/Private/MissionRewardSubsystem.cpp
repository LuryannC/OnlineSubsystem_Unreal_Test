// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#include "MissionRewardSubsystem.h"
#include "MissionRewardSave.h"
#include "MissionRewardSettings.h"
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
	if (!ActiveMissions.IsEmpty())
	{
		for (auto Mission : ActiveMissions)
		{
			Mission.Get()->OnProgressUpdated.RemoveAll(this);
			Mission.Get()->OnMissionCompleted.RemoveAll(this);
		}
	}
	
	ActiveMissions.Empty();
	ListenerMap.Empty();
	CompletedMissions.Empty();
	OnGoingMissionsProgress.Empty();

	OnMissionAdded.RemoveAll(this);
	OnMissionCompleted.RemoveAll(this);
	OnRewardReceived.RemoveAll(this);
	OnMissionRewardSystemLoaded.RemoveAll(this);
	OnMissionRewardSystemSaved.RemoveAll(this);
	
	Super::Deinitialize();
}

void UMissionRewardSubsystem::LoadProgress()
{
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (const UMissionRewardSave* Save = Cast<UMissionRewardSave>(UGameplayStatics::LoadGameFromSlot(Settings->SaveSlotName, Settings->SaveUserIndex)))
		{
			CompletedMissions = Save->CompletedMissions;
			OnGoingMissionsProgress = Save->OnGoingMissionsProgress;
			GrantedMissions = Save->GrantedMissions;
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
	
	CompletedMissions = SaveData->CompletedMissions;
	OnGoingMissionsProgress = SaveData->OnGoingMissionsProgress;
	GrantedMissions = SaveData->GrantedMissions;

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

		for (const TSoftObjectPtr<UMissionsAsset>& MissionAsset : Settings->Missions)
		{
			if (UMissionsAsset* CurrentMissionAsset = MissionAsset.LoadSynchronous())
			{
				Assets.Add(CurrentMissionAsset);
			}
		}
		
		PreInitMissionsFromAssets(Assets);
		LoadRuntimeGrantedMissions();
	}
}

void UMissionRewardSubsystem::PreInitMissionsFromAssets(const TArray<UMissionsAsset*>& Assets)
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

				const FGuid DeterministicGuID = FGuid::NewDeterministicGuid(LoadedClass->GetPathName());
				
				const bool bIsCompleted = CompletedMissions.ContainsByPredicate([&](const FCompletedMission& CurrentMission)
				{
					return (CurrentMission.InstanceID.IsValid() && CurrentMission.InstanceID == DeterministicGuID);
				});
				
				if (bIsCompleted)
				{
					DefaultMissionData->SetMissionCompleted();
				}
				else
				{
					if (UMissionBase* Mission = NewObject<UMissionBase>(this, LoadedClass))
					{
						Mission->SetInstanceID(DeterministicGuID);
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
	
	if (!InMission->OnProgressUpdated.IsBound())
	{
		InMission->OnProgressUpdated.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionProgressUpdated);
	}

	if (!InMission->OnMissionCompleted.IsBound())
	{
		InMission->OnMissionCompleted.AddDynamic(this, &UMissionRewardSubsystem::HandleMissionCompleted);
	}
		
	if (const FProgressedMissions* Saved = OnGoingMissionsProgress.FindByPredicate([&](const FProgressedMissions& Progress)
	{
		return Progress.InstanceID == InMission->GetMissionData().InstanceID;
	}))
		{
			InMission->UpdateRuntimeConditionsProgress(Saved->ConditionsProgress);
		};		

	ActiveMissions.Add(InMission);
	OnMissionAdded.Broadcast(InMission);
		
	for (const FMissionCondition& Condition : InMission->GetMissionData().Conditions)
	{
		ListenerMap.Add(Condition.EventTag, InMission);
	}
}

void UMissionRewardSubsystem::GrantMission(TSoftClassPtr<UMissionBase> MissionClass, const bool bAllowDuplicates)
{
	if (!MissionClass)
	{
		UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionRewardSubsystem::GrantMission - called with invalid MissionClass."));
		return;
	}

	const UMissionBase* Mission = Cast<UMissionBase>(MissionClass->GetDefaultObject());
	if (!Mission)
	{
		UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionRewardSubsystem::GrantMission - Could not get default object for mission."));
		return;
	}

	const FName MissionID = Mission->GetMissionData().MissionID;
	
	if (!bAllowDuplicates)
	{	
		if (ActiveMissions.ContainsByPredicate([&](const UMissionBase* M){ return M->GetMissionData().MissionID == MissionID; }))
		{
			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GrantMission - Mission %s already active."), *MissionID.ToString());
			return;
		}
	}

	if (UMissionBase* NewMission = NewObject<UMissionBase>(this, MissionClass.Get()))
	{
		InitMission(NewMission);

		const FMissionStruct NewProgress(NewMission->GetMissionData());
		OnGoingMissionsProgress.Add(NewProgress);
		GrantedMissions.Add(FGrantedMission(NewMission->GetClass(), NewMission->GetMissionData().InstanceID));

		CommitSave();

		UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionRewardSubsystem::GrantMission - Granted mission %s"), *MissionID.ToString());
	}
}

void UMissionRewardSubsystem::ReportGameplayEvent(const FGameplayTag& EventTag, const int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	
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

	CompletedMissions.Add(Mission->GetMissionData());
	ActiveMissions.Remove(Mission);
	
	OnMissionCompleted.Broadcast(Mission->GetMissionData().MissionID.ToString());

	const FGuid MissionInstanceID = Mission->GetMissionData().InstanceID;

	const int32 IndexToRemove = OnGoingMissionsProgress.IndexOfByPredicate([&](const FProgressedMissions& CurrentMission)
	{
		return CurrentMission.InstanceID == MissionInstanceID;
	});
	
	if (IndexToRemove != INDEX_NONE)
	{
		OnGoingMissionsProgress.RemoveAt(IndexToRemove);
	}

	// Check if it was a granted mission
	GrantedMissions.RemoveAll([&](const FGrantedMission& GrantedMission)
	{
		return GrantedMission.InstanceID == MissionInstanceID;
	});
	
	for (const FMissionCondition& Condition : Mission->GetMissionData().Conditions)
	{
		ListenerMap.RemoveSingle(Condition.EventTag, Mission);
	}
	
	GiveMissionRewards(Mission);

	Mission->OnProgressUpdated.RemoveAll(this);
	Mission->OnMissionCompleted.RemoveAll(this);

	CommitSave();
}

void UMissionRewardSubsystem::HandleMissionProgressUpdated(UMissionBase* Mission)
{
	if (!Mission) return;

	const FGuid MissionInstanceID = Mission->GetMissionData().InstanceID;
	const FMissionStruct NewProgress(Mission->GetMissionData());
	
	int32 ExistingIndex = OnGoingMissionsProgress.IndexOfByPredicate([&](const FProgressedMissions& Entry)
	{
		return Entry.InstanceID == MissionInstanceID;
	});

	if (ExistingIndex != INDEX_NONE)
	{
		OnGoingMissionsProgress[ExistingIndex] = NewProgress;
	}
	else
	{
		OnGoingMissionsProgress.Add(NewProgress);
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
			Save->CompletedMissions = CompletedMissions;
			Save->OnGoingMissionsProgress = OnGoingMissionsProgress;
			Save->GrantedMissions = GrantedMissions;

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

			RewardInstance->GrantReward();
			const FName MissionID = Mission->GetMissionData().MissionID;

			OnRewardReceived.Broadcast(MissionID.ToString());
		}
	}
}

void UMissionRewardSubsystem::LoadRuntimeGrantedMissions()
{
	for (const auto& GrantedClass : GrantedMissions)
	{
		if (const UClass* LoadedClass = GrantedClass.MissionClass.LoadSynchronous())
		{
			if (UMissionBase* Mission = NewObject<UMissionBase>(this, LoadedClass))
			{
				Mission->SetInstanceID(GrantedClass.InstanceID);

				const bool bIsCompleted = CompletedMissions.ContainsByPredicate([&](const FCompletedMission& C)
				{
					return (C.InstanceID.IsValid() && C.InstanceID == Mission->GetMissionData().InstanceID);
				});
				if (bIsCompleted)
				{
					Mission->SetMissionCompleted();
				}
				else
				{
					InitMission(Mission);
				}
			}
		}
	}
}
