// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "SpaceRocksGameState.generated.h"

/**
 * 
 */
UCLASS()
class SPACEROCKS_API ASpaceRocksGameState : public AGameState
{
public:
	GENERATED_UCLASS_BODY()

	// Begin AGameState overrides
	virtual void DefaultTimer() override;
	virtual void OnConstruction(const FTransform& Transform);
	// End AGameState overrides

	// Basic 3D Asteroids-Style Game State Parameters
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 num_levels;	// Number of skill levels per map
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		float spacerock_start_speed;	// Initial speed of space rocks at level 1
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 spacerock_speed_inc;		// Increment speed of space rocks per level
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 num_spacerocks_start;		// Initial number of space rocks at level 1
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 num_spacerocks_inc;		// Increment number of space rocks per level

	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		FString map_name;	// Name of current Map
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 curr_level;	// Current level on map
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		float curr_spacerock_speed;	// Current speed of spacerocks
	UPROPERTY(Category = SpaceRocksGameMode, EditAnywhere)
		int32 curr_spacerocks;	// Current number of spacerocks (to spawn on each level)

	UFUNCTION(BlueprintCallable, Category = SpaceRocksGameState)
		float GetSpacerockSpawnSpeed();

};
