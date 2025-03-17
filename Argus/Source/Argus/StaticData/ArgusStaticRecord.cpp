// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticRecord.h"
#include "Misc/Paths.h"
#include "UObject/ObjectSaveContext.h"
#include <filesystem>

void UArgusStaticRecord::PreSave(FObjectPreSaveContext saveContext)
{
#if WITH_EDITOR
	FString fullPath = FPaths::ConvertRelativePathToFull(saveContext.GetTargetFilename());
	if (!std::filesystem::exists(TCHAR_TO_UTF8(*fullPath)))
	{
		// TODO JAMES: We are saving this record for the first time! We should check to see if a reference to it exists. If not, we need
		// to retrieve the associated database and automatically save this record inside of it.
	}
#endif

	Super::PreSave(saveContext);
}