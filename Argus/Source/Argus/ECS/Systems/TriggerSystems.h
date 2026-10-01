// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

struct TriggerSystemsArgs;

class TriggerSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateCircleTriggerOverlaps(const TriggerSystemsArgs& components);
	static void UpdatePolygonTriggerOverlaps(const TriggerSystemsArgs& components);
};
