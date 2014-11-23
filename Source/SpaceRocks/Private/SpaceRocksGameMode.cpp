// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SpaceRocks.h"
#include "SpaceRocksGameMode.h"
#include "SpaceRocksGameState.h"
#include "SpaceRocksPawn.h"

ASpaceRocksGameMode::ASpaceRocksGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ASpaceRocksPawn::StaticClass();

	// set default gamestate class

	GameStateClass = ASpaceRocksGameState::StaticClass();

	
}
