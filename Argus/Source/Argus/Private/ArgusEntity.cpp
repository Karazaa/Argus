// Fill out your copyright notice in the Description page of Project Settings.

#include "ArgusEntity.h"

ArgusEntity::ArgusEntity(uint32_t id) : m_id(id)
{
}

ArgusEntity::~ArgusEntity()
{
}

uint32_t ArgusEntity::GetId()
{
	return m_id;
}
