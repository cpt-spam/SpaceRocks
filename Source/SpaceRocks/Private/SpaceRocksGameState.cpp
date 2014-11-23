// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceRocks.h"
#include "SpaceRocksGameState.h"
#include "SpaceRocksGameMode.h"


ASpaceRocksGameState::ASpaceRocksGameState(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Basic 3D Asteroids-Style Game State Parameters
	num_levels = 10;
	spacerock_start_speed = 1000;	// Initial speed of space rocks at level 1
	spacerock_speed_inc = 100;	// Increment speed of space rocks per level
	num_spacerocks_start = 2;	// Initial number of space rocks at level 1
	num_spacerocks_inc = 1;		// Increment number of space rocks per level
	
}

void ASpaceRocksGameState::OnConstruction(const FTransform& Transform)
{
	// Call Super class function
	Super::OnConstruction(Transform);

	map_name = GetWorld()->GetMapName();			// Map Name
	curr_level = 1;									// Current level on map
	curr_spacerock_speed = spacerock_start_speed;	// Current speed of spacerocks
	curr_spacerocks = num_spacerocks_start;			// Current number of spacerocks (to spawn on each level)

}

void ASpaceRocksGameState::DefaultTimer()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This is an on screen message!"));


	// Periodically do game related stuff (e.g. spawn stuff) 

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetWorld()->GetMapName());
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, map_name);

	Super::DefaultTimer();
}

float ASpaceRocksGameState::GetSpacerockSpawnSpeed()
{
	return curr_spacerock_speed;
}
