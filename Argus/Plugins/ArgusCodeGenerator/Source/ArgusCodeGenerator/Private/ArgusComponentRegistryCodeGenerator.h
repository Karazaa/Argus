// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include <string>
#include <vector>

class ArgusComponentRegistryCodeGenerator 
{
public:
	static void GenerateComponentRegistry();
private:
	static void ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames);
};