// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class ArgusEntity;

struct FogOfWarComponent;

class FogOfWarSystems
{
public:
	static void InitializeSystems();
	static void RunSystems(float deltaTime);

private:
	static void ClearRevealedPixels(FogOfWarComponent* fogOfWarComponent);
	static void SetRevealedPixels(FogOfWarComponent* fogOfWarComponent);
	static void RevealPixelsForEntity(FogOfWarComponent* fogOfWarComponent, const ArgusEntity& entity);
	static void UpdateTexture();

	static bool DoesPixelEqualColor(FogOfWarComponent* fogOfWarComponent, uint32 pixelNumber, FColor color);
};
