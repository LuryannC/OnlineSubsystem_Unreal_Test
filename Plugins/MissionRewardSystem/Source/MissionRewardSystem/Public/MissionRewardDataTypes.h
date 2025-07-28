#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissionRewardDataTypes.generated.h"

class URewardBase;

UENUM(BlueprintType)
enum EUnlockReasonFailReason : uint8
{
	NotFound,
	AlreadyCompleted,
	Unknown,
	Success,
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FMissionCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Event"))
	FGameplayTag EventTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TargetCount = 1;
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FRuntimeCondition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag = FGameplayTag();

	UPROPERTY(BlueprintReadOnly)
	int32 Target = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Current = 0;
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FMissionStruct
{
	GENERATED_BODY()

	FMissionStruct() {}
	FMissionStruct(const FName InMissionID, const TArray<FRuntimeCondition>& InConditionProgress)
	{
		MissionID = InMissionID;
		ConditionsProgress = InConditionProgress;
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MissionID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMissionCondition> Conditions;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards")
	// TArray<TSoftObjectPtr<URewardBase>> Rewards = TArray<TSoftObjectPtr<URewardBase>>();

	UPROPERTY(BlueprintReadOnly)
	TArray<FRuntimeCondition> ConditionsProgress;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	bool operator==(const FMissionStruct& Other) const
	{
		return MissionID == Other.MissionID;
	}

	
};