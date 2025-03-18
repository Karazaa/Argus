// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticRecord.h"
#include "ArgusStaticData.h"
#include "Misc/Paths.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectSaveContext.h"
#include <filesystem>

void UArgusStaticRecord::PreSave(FObjectPreSaveContext saveContext)
{
#if WITH_EDITOR
	FString fullPath = FPaths::ConvertRelativePathToFull(saveContext.GetTargetFilename());
	if (!std::filesystem::exists(TCHAR_TO_UTF8(*fullPath)))
	{
		TArray<FReferencerInformation> internalReferencers;
		TArray<FReferencerInformation> externalReferencers;
		RetrieveReferencers(&internalReferencers, &externalReferencers);

		if (internalReferencers.IsEmpty() && externalReferencers.IsEmpty())
		{
			// TODO JAMES: We are saving this record for the first time! We need
			// to retrieve the associated database and automatically save this record inside of it.
			ArgusStaticData::AddRecordToDatabase(this);
		}
	}
#endif

	Super::PreSave(saveContext);
}