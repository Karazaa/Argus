// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ArgusEntity
{
public:
	ArgusEntity(uint32_t id);
	~ArgusEntity();
	uint32_t GetId();

private:
	uint32_t m_id;
};
