// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaSettings.h"


UVlcMediaSettings::UVlcMediaSettings()
	: DiscCaching(FTimespan::FromMilliseconds(10000.0))
	, FileCaching(FTimespan::FromMilliseconds(10000.0))
	, LiveCaching(FTimespan::FromMilliseconds(10000.0))
	, NetworkCaching(FTimespan::FromMilliseconds(10000.0))
	, LogLevel(EVlcMediaLogLevel::Error)
	, ShowLogContext(false)
{ }
