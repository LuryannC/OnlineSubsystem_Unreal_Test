// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#include "MissionBase.h"
#include "MissionRewardSettings.h"
#include "MissionRewardSystemLog.h"

void UMissionBase::InitialiseMission()
{
	MissionData.bIsCompleted = false;

	// MissionID is mandatory as a lot of logic resolve around it.
	const FString MissionID = MissionData.MissionID.ToString();
	if (MissionID.IsEmpty())
	{
		UE_LOG(MissionRewardSystemLog, Error, TEXT("UMissionBase::InitialiseMission() - MissionID is empty!! Please give missions an ID."))
	}

	if (!MissionData.InstanceID.IsValid())
	{
		MissionData.InstanceID = FGuid::NewGuid();
	}
	
	ShowMissionDebugData();
}

FMissionStruct UMissionBase::GetMissionData() const
{
	return MissionData;
}

void UMissionBase::SetMissionCompleted()
{
	MissionData.bIsCompleted = true;
}

void UMissionBase::OnGameplayEvent(const FGameplayTag& EventTag, const int32 Amount)
{
	if (MissionData.bIsCompleted || Amount <= 0)
	{
		UE_LOG(MissionRewardSystemLog, Error, TEXT("UMissionBase::OnGameplayEvent - Amount can't be <= 0."));
		return;
	}

	bool bAnyProgressed = false;

	if (MissionData.Conditions.Num() == 0)
	{
		UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionBase::OnGameplayEvent - No runtime conditions found. Skipping progress."));
		return;
	}

	for (FMissionCondition& RC : MissionData.Conditions)
	{
		// Exact match is cheapest; Although MatchesTag allow for hierarchy support EventTag.MatchesTag(RC.EventTag)
		if (EventTag.MatchesTagExact(RC.EventTag))
		{
			const int32 Old = RC.Current;
			RC.Current = FMath::Clamp(RC.Current + Amount, 0, RC.TargetCount);
			if (RC.Current != Old)
			{
				bAnyProgressed = true;
			}
		}
	}
	

	if (!bAnyProgressed)
	{
		return;
	}

	bool bAllComplete = true;

	for (const FMissionCondition& RC : MissionData.Conditions)
	{
		if (RC.TargetCount <= 0)
		{
			UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionBase::OnGameplayEvent - Invalid RC.Target (%d) for tag %s"), RC.TargetCount, *RC.EventTag.ToString());
			bAllComplete = false;
			continue;
		}
		
		if (RC.Current < RC.TargetCount)
		{
			bAllComplete = false;
		}
	}

	OnProgressUpdated.Broadcast(this);
	
	if (bAllComplete)
	{
		MissionData.bIsCompleted = true;
		OnMissionCompleted.Broadcast(this);
	}

	ShowMissionDebugData();
}

void UMissionBase::UpdateRuntimeConditionsProgress(const TArray<FMissionCondition>& InMissionCondition)
{
	MissionData.Conditions = InMissionCondition;
	ShowMissionDebugData();
}

void UMissionBase::ShowMissionDebugData()
{
#if WITH_EDITOR
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (Settings->bShowDebugMessages)
		{
			UE_LOG(MissionRewardSystemLog, Log, TEXT("---------------- MISSION ----------------"));
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Instance ID: %s"), *MissionData.InstanceID.ToString());
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Mission ID: %s"), *MissionData.MissionID.ToString());
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Num of Conditions: %i"), MissionData.Conditions.Num());
			for (const auto& Condition : MissionData.Conditions)
			{
				UE_LOG(MissionRewardSystemLog, Log, TEXT("Condition: %s | Current: %i | Target: %i"), *Condition.EventTag.ToString(), Condition.Current, Condition.TargetCount);
			}
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Mission completed: %hs"), MissionData.bIsCompleted ? ("True") : ("False"));
			UE_LOG(MissionRewardSystemLog, Log, TEXT("-----------------------------------------"));
		}
	}
#endif
}


