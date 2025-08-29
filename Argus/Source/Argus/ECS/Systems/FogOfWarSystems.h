// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class ArgusEntity;

struct FogOfWarComponent;
struct FogOfWarSystemsArgs;

class FogOfWarSystems
{
public:
	static void InitializeSystems();
	static void RunSystems(float deltaTime);

private:
	static void ClearRevealedPixels(FogOfWarComponent* fogOfWarComponent);
	static void SetRevealedPixels(FogOfWarComponent* fogOfWarComponent);
	static void RevealPixelsForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components);
	static void UpdateTexture();

	static bool DoesPixelEqualColor(FogOfWarComponent* fogOfWarComponent, uint32 pixelNumber, FColor color);
	static uint32 GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation);
	static uint32 GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius);
};
