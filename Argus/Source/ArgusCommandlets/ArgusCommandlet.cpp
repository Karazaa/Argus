// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCommandlet.h"
#include "PackageHelperFunctions.h"
#include "SourceControlHelpers.h"

UArgusCommandlet::UArgusCommandlet()
{
	IsClient = true;
	IsEditor = true;
	IsServer = true;
	LogToConsole = true;
}

int32 UArgusCommandlet::Main(const FString& parameters)
{
	OnStart();
	int32 result = DoWork();
	OnFinish();

	return result;
}

bool UArgusCommandlet::SaveDataAsset(const UDataAsset* dataAssetToSave) const
{
	if (!dataAssetToSave)
	{
		return false;
	}

	UPackage* package = dataAssetToSave->GetPackage();
	if (!ensure(package))
	{
		return false;
	}

	const FString packageFilename = SourceControlHelpers::PackageFilename(package);
	return SavePackageHelper(package, packageFilename);
}
