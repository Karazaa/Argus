// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "SystemArgumentDefinitions/FogOfWarSystemsArgs.h"

void FogOfWarSystems::InitializeSystems()
{
	ARGUS_TRACE(FogOfWarSystems::InitializeSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture = UTexture2D::CreateTransient(fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize, PF_A8);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	fogOfWarComponent->m_fogOfWarTexture->SRGB = 0;
	fogOfWarComponent->m_fogOfWarTexture->Filter = TextureFilter::TF_Nearest;
	fogOfWarComponent->m_fogOfWarTexture->AddToRoot();
	fogOfWarComponent->m_fogOfWarTexture->UpdateResource();
	fogOfWarComponent->m_textureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize);

	fogOfWarComponent->m_textureData.SetNumZeroed(fogOfWarComponent->GetTotalPixels());
	memset(fogOfWarComponent->m_textureData.GetData(), 255, fogOfWarComponent->GetTotalPixels());
}

void FogOfWarSystems::RunThreadSystems()
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Iterate over all entities and carve out a circle of pixels (activelyRevealed) based on sight radius for entities that are on the local player team (or allies).
	SetRevealedStatePerEntity(fogOfWarComponent);
}

void FogOfWarSystems::RunSystems()
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	UpdateTexture();
	UpdateDynamicMaterialInstance();
}

void FogOfWarSystems::SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::SetRevealedPixels);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	// Set actively revealed pixels to revealed once.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam))
		{
			continue;
		}

		const uint32 centerPixel = GetPixelNumberFromWorldSpaceLocation(fogOfWarComponent, components.m_transformComponent->m_location);
		const bool newCenterPixel = centerPixel != components.m_transformComponent->m_fogOfWarPixel;
		if ((newCenterPixel || !entity.IsAlive()) && components.m_transformComponent->m_fogOfWarPixel != MAX_uint32)
		{
			RevealPixelAlphaForEntity(fogOfWarComponent, components, false);
		}

		if (!entity.IsAlive())
		{
			components.m_transformComponent->m_fogOfWarPixel = MAX_uint32;
			continue;
		}
		components.m_transformComponent->m_fogOfWarPixel = centerPixel;
	}

	// Calculate new actively revealed pixels.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!DoesEntityNeedToUpdateActivelyRevealed(components, inputInterfaceComponent))
		{
			continue;
		}

		RevealPixelAlphaForEntity(fogOfWarComponent, components, true);
	}
}

void FogOfWarSystems::RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelsForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint32 textureSize = static_cast<uint32>(fogOfWarComponent->m_textureSize);
	if (textureSize == 0u)
	{
		return;
	}

	const uint32 maxPixel = fogOfWarComponent->GetTotalPixels() - 1;
	const uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	const uint32 centerPixelRowNumber = components.m_transformComponent->m_fogOfWarPixel / textureSize;

	FogOfWarOffsets fogOfWarOffsets;
	fogOfWarOffsets.leftOffset = radius;
	// The radius overlaps the left edge of the texture.
	if (((components.m_transformComponent->m_fogOfWarPixel - fogOfWarOffsets.leftOffset) / textureSize) != centerPixelRowNumber)
	{
		fogOfWarOffsets.leftOffset = components.m_transformComponent->m_fogOfWarPixel % textureSize;
	}

	fogOfWarOffsets.rightOffset = radius;
	// The radius overlaps the right edge of the texture.
	if (((components.m_transformComponent->m_fogOfWarPixel + fogOfWarOffsets.rightOffset) / textureSize) != centerPixelRowNumber)
	{
		fogOfWarOffsets.rightOffset = textureSize - (components.m_transformComponent->m_fogOfWarPixel % textureSize);
	}

	fogOfWarOffsets.topOffset = radius;
	// The radius overlaps the top edge of the texture.
	if (fogOfWarOffsets.topOffset > centerPixelRowNumber)
	{
		fogOfWarOffsets.topOffset = centerPixelRowNumber;
	}

	fogOfWarOffsets.bottomOffset = radius;
	// The radius overlaps the bottom edge of the texture.
	if ((fogOfWarOffsets.bottomOffset + centerPixelRowNumber) >= textureSize)
	{
		fogOfWarOffsets.bottomOffset = ((maxPixel - (textureSize - (components.m_transformComponent->m_fogOfWarPixel % textureSize))) / textureSize) - (centerPixelRowNumber);
	}

	// Method of Horn for circle rasterization.
	int32 circleD = -static_cast<int32>(radius);
	fogOfWarOffsets.circleX = radius;
	fogOfWarOffsets.circleY = 0u;
	while (fogOfWarOffsets.circleX >= fogOfWarOffsets.circleY)
	{
		// Set Alpha for pixel range for all symmetrical pixels.
		SetAlphaForCircleOctant(fogOfWarComponent, components, fogOfWarOffsets, activelyRevealed);

		circleD = circleD + (2 * fogOfWarOffsets.circleX * fogOfWarOffsets.circleY) + 1u;
		fogOfWarOffsets.circleY++;

		if (circleD > 0)
		{
			circleD = circleD - (2 * fogOfWarOffsets.circleX * fogOfWarOffsets.circleX) + 2;
			fogOfWarOffsets.circleX--;
		}
	}
}


void FogOfWarSystems::SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForPixelRange);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	uint8* firstAddress = &fogOfWarComponent->m_textureData[fromPixelInclusive];
	uint32 rowLength = (toPixelInclusive - fromPixelInclusive);
	memset(firstAddress, activelyRevealed ? 0 : fogOfWarComponent->m_revealedOnceAlpha, rowLength);
}

void FogOfWarSystems::SetAlphaForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& fogOfWarOffsets, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForCircleOctant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint32 topY = fogOfWarOffsets.circleX <= fogOfWarOffsets.topOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.topOffset;
	const uint32 topStartX = fogOfWarOffsets.circleY <= fogOfWarOffsets.leftOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.leftOffset;
	const uint32 topEndX = fogOfWarOffsets.circleY <= fogOfWarOffsets.rightOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.rightOffset;

	const uint32 bottomY = fogOfWarOffsets.circleX <= fogOfWarOffsets.bottomOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.bottomOffset;
	const uint32 bottomStartX = fogOfWarOffsets.circleY <= fogOfWarOffsets.leftOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.leftOffset;
	const uint32 bottomEndX = fogOfWarOffsets.circleY <= fogOfWarOffsets.rightOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.rightOffset;

	const uint32 midUpY = fogOfWarOffsets.circleY <= fogOfWarOffsets.topOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.topOffset;
	const uint32 midUpStartX = fogOfWarOffsets.circleX <= fogOfWarOffsets.leftOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.leftOffset;
	const uint32 midUpEndX = fogOfWarOffsets.circleX <= fogOfWarOffsets.rightOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.rightOffset;

	const uint32 midDownY = fogOfWarOffsets.circleY <= fogOfWarOffsets.bottomOffset ? fogOfWarOffsets.circleY : fogOfWarOffsets.bottomOffset;
	const uint32 midDownStartX = fogOfWarOffsets.circleX <= fogOfWarOffsets.leftOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.leftOffset;
	const uint32 midDownEndX = fogOfWarOffsets.circleX <= fogOfWarOffsets.rightOffset ? fogOfWarOffsets.circleX : fogOfWarOffsets.rightOffset;

	const uint32 textureSize = static_cast<uint32>(fogOfWarComponent->m_textureSize);
	const uint32 centerColumnTopIndex = components.m_transformComponent->m_fogOfWarPixel - (topY * textureSize);
	const uint32 centerColumnMidUpIndex = components.m_transformComponent->m_fogOfWarPixel - (midUpY * textureSize);
	const uint32 centerColumnMidDownIndex = components.m_transformComponent->m_fogOfWarPixel + (midDownY * textureSize);
	const uint32 centerColumnBottomIndex = components.m_transformComponent->m_fogOfWarPixel + (bottomY * textureSize);

	SetAlphaForPixelRange(fogOfWarComponent, centerColumnTopIndex - topStartX, centerColumnTopIndex + topEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, centerColumnMidUpIndex - midUpStartX, centerColumnMidUpIndex + midUpEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, centerColumnMidDownIndex - midDownStartX, centerColumnMidDownIndex + midDownEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, centerColumnBottomIndex - bottomStartX, centerColumnBottomIndex + bottomEndX, activelyRevealed);
}

void FogOfWarSystems::UpdateTexture()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateTexture);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData = TextureRegionsUpdateData();

	FTextureResource* textureResource = fogOfWarComponent->m_fogOfWarTexture->GetResource();
	if (!textureResource)
	{
		return;
	}

	fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource = reinterpret_cast<FTexture2DResource*>(textureResource);
	fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI = fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource->GetTexture2DRHI();
	fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex = 0;
	fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_regions = &fogOfWarComponent->m_textureRegion;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch = fogOfWarComponent->m_textureSize;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_textureData.GetData();

	if (!fogOfWarComponent->m_textureRegionsUpdateData.m_srcData)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[fogOfWarComponent](FRHICommandListImmediate& RHICmdList)
		{
			ARGUS_TRACE(FogOfWarSystems::ExecuteTextureUpdate);
			for (uint32 regionIndex = 0; regionIndex < fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions; ++regionIndex)
			{
				int32 currentFirstMip = fogOfWarComponent->m_fogOfWarTexture->FirstResourceMemMip;
				if (fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI && fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex >= currentFirstMip)
				{
					RHIUpdateTexture2D(
						fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI,
						fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex - currentFirstMip,
						fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex],
						fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch,
						fogOfWarComponent->m_textureRegionsUpdateData.m_srcData
						+ fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex].SrcY * fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch
						+ fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex].SrcX * fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp
					);
				}
			}
		});
}

void FogOfWarSystems::UpdateDynamicMaterialInstance()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateDynamicMaterialInstance);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Set the dynamic material instance texture param to the fog of war texture.
	if (fogOfWarComponent->m_dynamicMaterialInstance && fogOfWarComponent->m_fogOfWarTexture)
	{
		fogOfWarComponent->m_dynamicMaterialInstance->SetTextureParameterValue("DynamicTexture", fogOfWarComponent->m_fogOfWarTexture);
	}
}

uint32 FogOfWarSystems::GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation)
{
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog);

	const float textureSize = static_cast<float>(fogOfWarComponent->m_textureSize);
	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;

	float xValue = ArgusMath::SafeDivide(worldSpaceLocation.Y + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;
	float yValue = ArgusMath::SafeDivide((-worldSpaceLocation.X) + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;

	uint32 xValue32 = static_cast<uint32>(FMath::FloorToInt32(xValue));
	uint32 yValue32 = static_cast<uint32>(FMath::FloorToInt32(yValue));

	return (yValue32 * static_cast<uint32>(fogOfWarComponent->m_textureSize)) + xValue32;
}

uint32 FogOfWarSystems::GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius)
{
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog);

	const float portion = ArgusMath::SafeDivide(radius, (2.0f * spatialPartitioningComponent->m_validSpaceExtent));
	return FMath::FloorToInt32(static_cast<float>(fogOfWarComponent->m_textureSize) * portion);
}

bool FogOfWarSystems::DoesEntityNeedToUpdateActivelyRevealed(const FogOfWarSystemsArgs& components, const InputInterfaceComponent* inputInterfaceComponent)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}
	ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);

	if (!components.m_entity.IsAlive() || !components.m_entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || components.m_entity.IsPassenger())
	{
		return false;
	}

	if (const VelocityComponent* velocityComponent = components.m_entity.GetComponent<VelocityComponent>())
	{
		if (!velocityComponent->m_currentVelocity.IsNearlyZero() || !velocityComponent->m_proposedAvoidanceVelocity.IsNearlyZero())
		{
			return true;
		}
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(spatialPartitioningComponent, ArgusECSLog);

	const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter = [&components, inputInterfaceComponent](const ArgusEntityKDTreeNode* entityNode)
	{
		ARGUS_RETURN_ON_NULL_BOOL(entityNode, ArgusECSLog);
		ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);
		if (entityNode->m_entityId == components.m_entity.GetId())
		{
			return false;
		}

		ArgusEntity otherEntity = ArgusEntity::RetrieveEntity(entityNode->m_entityId);
		if (!otherEntity || !otherEntity.IsAlive() || !otherEntity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || otherEntity.IsPassenger())
		{
			return false;
		}

		if (const VelocityComponent* velocityComponent = otherEntity.GetComponent<VelocityComponent>())
		{
			if (!velocityComponent->m_currentVelocity.IsNearlyZero() || !velocityComponent->m_proposedAvoidanceVelocity.IsNearlyZero())
			{
				return true;
			}
		}

		return false;
	};

	uint16 nearestMovingTeammateId = spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdClosestToArgusEntity(components.m_entity, queryFilter);
	if (nearestMovingTeammateId == ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	FogOfWarSystemsArgs otherComponents;
	if (!otherComponents.PopulateArguments(ArgusEntity::RetrieveEntity(nearestMovingTeammateId)))
	{
		return false;
	}

	const float distSquared = FVector::DistSquared(components.m_transformComponent->m_location, otherComponents.m_transformComponent->m_location);
	const float radiusSquared = FMath::Square(components.m_targetingComponent->m_sightRange + otherComponents.m_targetingComponent->m_sightRange);

	// TODO JAMES: We need a way to handle the case where an overlapping entity dies.
	return distSquared < radiusSquared;
}
