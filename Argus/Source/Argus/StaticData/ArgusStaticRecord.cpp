// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticRecord.h"
#include "ArgusMacros.h"

#if WITH_EDITOR
#include "ArgusStaticData.h"
#include "Misc/Paths.h"
#include "PackageHelperFunctions.h"
#include "SourceControlHelpers.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectSaveContext.h"
#include <filesystem>

void UArgusStaticRecord::PreSave(FObjectPreSaveContext saveContext)
{
	Super::PreSave(saveContext);
	if (saveContext.IsCooking() || saveContext.GetTargetPlatform() || m_id != 0u)
	{
		return;
	}

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

bool UArgusStaticRecord::SaveRecord() const 
{
	UPackage* package = GetPackage();
	ARGUS_RETURN_ON_NULL_BOOL(package, ArgusStaticDataLog);

	const FString packageFilename = SourceControlHelpers::PackageFilename(package);
	return SavePackageHelper(package, packageFilename);
}
#endif //WITH_EDITOR