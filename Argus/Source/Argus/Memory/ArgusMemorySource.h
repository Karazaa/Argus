// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

/*
 *
 * Arena to be used for all of the dynamic allocations in the gameplay logic of Argus. I can't control what the engine does with its allocations¯\_(ツ)_/¯
 *
 */

class ArgusMemorySource
{
public:
	

private:
	static void* m_sRawDataRoot;
};