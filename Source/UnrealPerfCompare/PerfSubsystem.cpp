#include "PerfSubsystem.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DEFINE_LOG_CATEGORY(LogPerfCompare);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static bool ErroredInternal = false;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if PLATFORM_WINDOWS
#include <Windows.h>
#include <bitset>
static TArray<FCacheInfo> GetCacheInfo()
{
	TArray<FCacheInfo> Output;
	DWORD BufferSize = 0u;
	
	using PInfo = SYSTEM_LOGICAL_PROCESSOR_INFORMATION;
	using CInfo = CACHE_DESCRIPTOR;
	using UPtr = std::unique_ptr<PInfo, decltype(&FMemory::Free)>;

	GetLogicalProcessorInformation(nullptr, &BufferSize);
	UPtr Buffer(static_cast<PInfo*>(FMemory::Malloc(BufferSize)), &FMemory::Free);
	if (!GetLogicalProcessorInformation(Buffer.get(), &BufferSize))
	{
		UE_LOG(LogPerfCompare, Fatal, TEXT("Failed during call to GetLogicalProcessorInformation: %u"), GetLastError());
		ErroredInternal = true;
		return Output;
	}

	for (PInfo *It = Buffer.get(), *Et = Buffer.get() + (BufferSize / sizeof(PInfo)); It != Et; ++It)
	{
		if (It->Relationship != RelationCache)
		{
			continue;
		}
		using BitSet = std::bitset<sizeof(ULONG_PTR) * CHAR_BIT>;
		BitSet BitArray(It->ProcessorMask);
		
		// To prevent duplicates, only consider caches where CPU 0 is specified
		if (!BitArray.test(0))
		{
			continue;
		}
		const CInfo& Cache = It->Cache;
		
		FCacheInfo C;
		C.NumSharing = static_cast<int32>(BitArray.count());
		C.Level = Cache.Level;
		C.Size = Cache.Size;
		C.Type = TEXT("Unknown");
		switch (Cache.Type)
		{
		case CacheUnified:
			C.Type = TEXT("Unified");
			break;
		case CacheInstruction:
			C.Type = TEXT("Instruction");
			break;
		case CacheData:
			C.Type = TEXT("Data");
			break;
		case CacheTrace:
			C.Type = TEXT("Trace");
			break;
		}
		Output.Emplace(MoveTemp(C));
	}

	return Output;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static TArray<FCacheInfo> CacheInfos;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void GlobalInitialize()
{
	CacheInfos = GetCacheInfo();
}
void GlobalCleanup()
{
	CacheInfos.Empty();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void Ready(UWorld* World)
{
	
}
static void Logic(UWorld* World)
{
	
}
static void End(UWorld* World)
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UPerfSubsystem::UPerfSubsystem()
	: Super()
	, Counter(0u)
{}
UPerfSubsystem::UPerfSubsystem(FVTableHelper& Helper)
	: Super(Helper)
	, Counter(0u)
{}


void UPerfSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Counter = 0u;
}
void UPerfSubsystem::Deinitialize()
{}


#if UE_EDITOR
ETickableTickType UPerfSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}
bool UPerfSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	switch (WorldType)
	{
	case EWorldType::Game:
	case EWorldType::PIE:
		return true;

	default:
		return false;
	}
}
bool UPerfSubsystem::IsTickableInEditor() const
{
	if (!GetWorld())
	{
		return false;
	}

	return true;
}
#endif

UWorld* UPerfSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}
TStatId UPerfSubsystem::GetStatId()const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPerfSubsystem, STATGROUP_Tickables);
}


void UPerfSubsystem::Tick(float DeltaSeconds)
{
	if (ErroredInternal)
	{
		FGenericPlatformMisc::RequestExit(false);
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
#if UE_EDITOR
	if (World->bDebugPauseExecution)
	{
		DeltaSeconds = 0.f;
	}
#endif

	switch(Counter++)
	{
	case 0u:
		Ready(World);
		break;

	case 1u:
		Logic(World);
		break;

	case 2u:
		End(World);
		FGenericPlatformMisc::RequestExit(false);
		break;

	default:
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

