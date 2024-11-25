// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusLogging.h"
#include "ArgusTesting.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY(ArgusECSLog);
DEFINE_LOG_CATEGORY(ArgusInputLog);
DEFINE_LOG_CATEGORY(ArgusStaticDataLog);
DEFINE_LOG_CATEGORY(ArgusUnrealObjectsLog);
DEFINE_LOG_CATEGORY(ArgusTestingLog);

void ArgusLogging::ShowEditorNotification(const FString& text, const FString& subText, ELogVerbosity::Type logVerbosity, float durationSeconds)
{
#if WITH_AUTOMATION_TESTS
	if (ArgusTesting::IsInTestingContext())
	{
		return;
	}
#endif //WITH_AUTOMATION_TESTS

#if WITH_EDITOR
	if (GIsEditor)
	{
		FName brushName;
		switch (logVerbosity)
		{
			case ELogVerbosity::Display:
				return;
			case ELogVerbosity::Log:
				return;
			case ELogVerbosity::Warning:
				brushName = TEXT("MessageLog.Warning");
				break;
			case ELogVerbosity::Error:
				brushName = TEXT("MessageLog.Error");
				break;
			case ELogVerbosity::Fatal:
				brushName = TEXT("MessageLog.Error");
				break;
		}

		FNotificationInfo notificationInfo(FText::FromString(text));
		notificationInfo.SubText = FText::FromString(subText);
		notificationInfo.ExpireDuration = durationSeconds;

		if (!brushName.IsNone())
		{
			notificationInfo.Image = FAppStyle::GetBrush(brushName);
		}

		FSlateNotificationManager::Get().AddNotification(notificationInfo);
	}
#endif // WITH_EDITOR
}