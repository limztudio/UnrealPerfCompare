#pragma once


#include "CoreMinimal.h"

#include "Tickable.h"
#include "Subsystems/WorldSubsystem.h"

#include "PerfSubsystem.generated.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DECLARE_LOG_CATEGORY_EXTERN(LogPerfCompare, Log, All);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FCacheInfo
{
	FString Type;
	int32 Level;
	int32 Size;
	int32 NumSharing;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UCLASS(Transient)
class UNREALPERFCOMPARE_API UPerfSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()


public:
	UPerfSubsystem();
	UPerfSubsystem(FVTableHelper& Helper);


public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


public:
#if UE_EDITOR
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual bool IsTickableInEditor() const override;
#endif

	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override;

	virtual void Tick(float DeltaSeconds) override;

	
private:
	uint32 Counter;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

