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
		uint32 m_leftOffset = 0u;
		uint32 m_rightOffset = 0u;
		uint32 m_topOffset = 0u;
		uint32 m_bottomOffset = 0u;

		uint32 m_circleX = 0u;
		uint32 m_circleY = 0u;
	};

	struct CircleOctantExpansion
	{
		uint32 m_topY = 0u;
		uint32 m_topStartX = 0u;
		uint32 m_topEndX =  0u;

		uint32 m_bottomY = 0u;
		uint32 m_bottomStartX = 0u;
		uint32 m_bottomEndX = 0u;

		uint32 m_midUpY = 0u;
		uint32 m_midUpStartX = 0u;
		uint32 m_midUpEndX = 0u;

		uint32 m_midDownY = 0u;
		uint32 m_midDownStartX = 0u;
		uint32 m_midDownEndX = 0u;

		uint32 m_centerColumnTopIndex = 0u;
		uint32 m_centerColumnMidUpIndex = 0u;
		uint32 m_centerColumnMidDownIndex = 0u;
		uint32 m_centerColumnBottomIndex = 0u;
	};

	static void InitializeGaussianFilter(FogOfWarComponent* fogOfWarComponent);
	static void SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent);
	static void PopulateOffsetsForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& outOffsets);
	static void PopulateOctantExpansionForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, CircleOctantExpansion& outCircleOctantExpansion);
	static void RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& offsets, bool activelyRevealed);
	static void BlurBoundariesForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& offsets);
	static void RasterizeCircleOfRadius(uint32 radius, FogOfWarOffsets& offsets, TFunction<void (const FogOfWarOffsets& offsets)> perOctantPixelFunction);
	static void SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, bool activelyRevealed);
	static void SetAlphaForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, bool activelyRevealed);
	static void BlurBoundariesForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets);
	static void UpdateTexture();
	static void UpdateDynamicMaterialInstance();

	static uint32 GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation);
	static uint32 GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius);
	static void UpdateDoesEntityNeedToUpdateActivelyRevealed(const FogOfWarSystemsArgs& components, const InputInterfaceComponent* inputInterfaceComponent);
	static bool IsPixelInFogOfWarBounds(int32 relativeX, int32 relativeY, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components);
	static void BlurAroundPixel(int32 relativeX, int32 relativeY, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components);

	static const float k_gaussianFilter[9];
};
