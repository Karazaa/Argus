// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCommandlet.h"
#include "Editor.h"
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

	UWorld* world = UWorld::FindWorldInPackage(worldPackage);
	if (!world)
	{
		return nullptr;
	}

	world->WorldType = EWorldType::Editor;
	world->AddToRoot();

	if (!world->bIsWorldInitialized)
	{
		world->InitWorld();
		if (world->PersistentLevel)
		{
			world->PersistentLevel->UpdateModelComponents();
		}
		world->UpdateWorldComponents(true, false);
	}

	FWorldContext& worldContext = GEditor->GetEditorWorldContext(true);
	if (UWorld* currentEditorWorld = worldContext.World())
	{
		if (currentEditorWorld->IsInitialized())
		{
			currentEditorWorld->ClearWorldComponents();
			currentEditorWorld->DestroyWorld(true);
			currentEditorWorld->RemoveFromRoot();
		}
	}

	worldContext.SetCurrentWorld(world);
	GWorld = world;
	m_currentlyLoadedWorld = world;

	return world;
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
