// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSettings.h"

#define LOCTEXT_NAMESPACE "ArgusSettings"

FName UArgusSettings::GetCategoryName() const
{
	return TEXT("Argus");
}

#if WITH_EDITOR
FText UArgusSettings::GetSectionText() const
{
	return LOCTEXT("DataLookupsName", "Data Lookups");
}

FText UArgusSettings::GetSectionDescription() const
{
	return  LOCTEXT("DataLookupsDescription", "References to Data that can be looked up in Editor or at Runtime.");
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE