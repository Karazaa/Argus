// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "AssetLoadingComponentData.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"

void UAssetLoadingComponentData::InstantiateComponentForEntity(const ArgusEntity& entity) const
{
	AssetLoadingComponent* AssetLoadingComponentRef = entity.AddComponent<AssetLoadingComponent>();
	ARGUS_RETURN_ON_NULL(AssetLoadingComponentRef, ArgusECSLog);

}

bool UAssetLoadingComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UAssetLoadingComponentData* otherComponentData = Cast<UAssetLoadingComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}
