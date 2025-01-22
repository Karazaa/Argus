// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class ArgusEntity;

class IArgusActorWidgetInterface
{
public:
	virtual void SetInitialDisplay(ArgusEntity& argusEntity) = 0;
	virtual void RefreshDisplay(ArgusEntity& argusEntity) = 0;
};