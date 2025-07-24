#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MissionRewardDataTypes.generated.h"

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

	UPROPERTY(BlueprintReadOnly)
	FName MissionID = FName();

	UPROPERTY(BlueprintReadOnly)
	TArray<FRuntimeCondition> ConditionsProgress = TArray<FRuntimeCondition>();

	bool operator==(const FMissionStruct& Other) const
	{
		return MissionID == Other.MissionID;
	}
};