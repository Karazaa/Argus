// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCommandlet.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
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

UWorld* UArgusCommandlet::LoadWorld(const FString& worldLongPackageName)
{
	if (m_currentlyLoadedWorld)
	{
		if (const UPackage* package = m_currentlyLoadedWorld->GetPackage())
		{
			if (worldLongPackageName.Equals(package->GetName()))
			{
				return m_currentlyLoadedWorld.Get();
			}
		}
	}

	UPackage* worldPackage = FindPackage(nullptr, *worldLongPackageName);
	if (!worldPackage)
	{
		worldPackage = LoadPackage(nullptr, *worldLongPackageName, LOAD_None);
	}

	// TODO JAMES: Need way of loading level as a UWorld for the Commandlet.
	// Map soft reference is stored in WorldCellRecord.
	// 
	// 1) X
	// 2) UWorld::FindWorldInPackage
	// 3) World->WorldType = EWorldType::Editor;
	// 4) World->AddToRoot
	// 5) Initialize World
	// 

	return nullptr;
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
