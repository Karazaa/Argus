// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class ArgusEntity;

struct FogOfWarComponent;
struct FogOfWarSystemsArgs;
struct InputInterfaceComponent;

class FogOfWarSystems
{
public:
	static void InitializeSystems();
	static void RunSystems();
	static void RunThreadSystems();

private:
	struct FogOfWarOffsets
	{
		uint32 leftOffset = 0u;
		uint32 rightOffset = 0u;
		uint32 topOffset = 0u;
		uint32 bottomOffset = 0u;
		uint32 circleX = 0u;
		uint32 circleY = 0u;
	};

	static void SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent);
	static void RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, bool activelyRevealed);
	static void RasterizeCircleOfRadius(uint32 radius, FogOfWarOffsets& offsets, TFunction<void (const FogOfWarOffsets& offsets)> perOctantPixelFunction);
	static void SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, bool activelyRevealed);
	static void SetAlphaForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, bool activelyRevealed);
	static void UpdateTexture();
	static void UpdateDynamicMaterialInstance();

	static uint32 GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation);
	static uint32 GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius);
	static bool DoesEntityNeedToUpdateActivelyRevealed(const FogOfWarSystemsArgs& components, const InputInterfaceComponent* inputInterfaceComponent);
};
