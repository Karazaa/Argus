// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

struct TimerComponent;

class TimerSystems
{
public:
	static void RunSystems(float deltaTime);
	static void AdvaceTimers(TimerComponent* timerComponent, float deltaTime);
};