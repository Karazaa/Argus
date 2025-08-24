// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class FogOfWarSystems
{
public:
	static void InitializeSystems();
	static void RunSystems(float deltaTime);

private:
	static void UpdateTexture();
};
