// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

struct WorldCellSystemsArgs;

class WorldCellSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateWorldCellLocationPerEntity(const WorldCellSystemsArgs& components);
};
