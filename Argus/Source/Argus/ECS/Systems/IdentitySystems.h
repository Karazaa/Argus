// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class IdentitySystems
{
public:
	// static void RunSystems(float deltaTime);

	static void RegisterEntityAsSeenByOther(const uint16 perceivedEntityId, const uint16 perceiverEntityId);
};
