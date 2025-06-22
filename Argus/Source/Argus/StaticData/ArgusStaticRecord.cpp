// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticRecord.h"
#include "ArgusMacros.h"

#if WITH_EDITOR && !IS_PACKAGING_ARGUS
#include "ArgusStaticData.h"
#include "Misc/Paths.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectSaveContext.h"
#include <filesystem>

void UArgusStaticRecord::PreSave(FObjectPreSaveContext saveContext)a
{
	FString fullPath = FPaths::ConvertRelativePathToFull(saveContext.GetTargetFilename());
	if (!std::filesystem::exists(TCHAR_TO_UTF8(*fullPath)))
	{
		TArray<FReferencerInformation> internalReferencers;
		TArray<FReferencerInformation> externalReferencers;
		RetrieveReferencers(&internalReferencers, &externalReferencers);

		if (internalReferencers.IsEmpty() && externalReferencers.IsEmpty())
		{
			ArgusStaticData::AddRecordToDatabase(this);
		}
	}

	Super::PreSave(saveContext);
}
#endif //WITH_EDITOR && !IS_PACKAGING_ARGUS