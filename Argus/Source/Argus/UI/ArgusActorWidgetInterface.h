// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class ArgusEntity;

class IArgusActorWidgetInterface
{
public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) = 0;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) = 0;
};