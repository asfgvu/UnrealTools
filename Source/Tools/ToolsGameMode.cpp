// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToolsGameMode.h"
#include "ToolsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AToolsGameMode::AToolsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
