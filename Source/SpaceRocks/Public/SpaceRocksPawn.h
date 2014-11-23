// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "SpaceRocksPawn.generated.h"

UCLASS(config=Game)
class ASpaceRocksPawn : public APawn
{
public:
	GENERATED_UCLASS_BODY()

	// StaticMesh component that will be the visuals for our flying pawn
	UPROPERTY(Category = SpaceRocksPawn, VisibleAnywhere, BlueprintReadOnly)
		TSubobjectPtr<class UStaticMeshComponent> PlaneMesh;

	// Sphere Mesh that will form bother the root component, plus a glowing sheild effect when hit
	// i.e. normally not visible.
	UPROPERTY(Category = SpaceRocksPawn, VisibleAnywhere, BlueprintReadOnly)
		TSubobjectPtr<class UStaticMeshComponent> ShieldMesh;

	// Spring arm that will offset the camera
	UPROPERTY(Category = SpaceRocksPawn, VisibleAnywhere, BlueprintReadOnly)
		TSubobjectPtr<class USpringArmComponent> SpringArm;

	// Third Person Camera component
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		TSubobjectPtr<class UCameraComponent> TP_Camera;

	// First Person Camera component
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		TSubobjectPtr<class UCameraComponent> FP_Camera;

	// Spot light
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		TSubobjectPtr<class USpotLightComponent> CraftSpotLight;



	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void ReceiveActorBeginOverlap(class AActor * Other) override;
	// End AActor overrides


	// Current XAxis speed
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentXAxisSpeed;

	// Current yaw speed
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentYawSpeed;

	// Current pitch speed
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentPitchSpeed;

	// Current roll speed 
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentRollSpeed;

	// Current ZAxis speed
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentZAxisSpeed;

	// Current YAxis speed
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float CurrentYAxisSpeed;


	// Weapon Handling

	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		int32 weapon;
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float lastfired;
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		bool primary_on;
	// Cycle through weapon fire positions
	int32 weap_cycle;

	// Pickup Handling
	UFUNCTION(BlueprintCallable, Category = SpaceRocksPawn)
		bool LookForInv(int32 PUtype);

	// Behaviour
	UPROPERTY(Category = SpaceRocksPawn, VisibleDefaultsOnly, BlueprintReadOnly)
		float ShieldLevel;

	// Inventories
	//UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly)
	//ReconOneInventory PlayerInv;
	//SpaceRocksInventory PlayerInv;

	// Helpers
	//UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly)
	//ReconOneWeaponInfo WeapInfo;
	//SpaceRocksWeaponInfo WeapInfo;

protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	virtual void OnConstruction(const FTransform& Transform);
	// End APawn overrides

	/** Craft Control Fuctions
	Orientation Trusters:
	- Pitch
	- Yaw
	- Roll
	Directional Trusters:
	- RearThrust
	- SideThrust
	- BottomThrust
	*/
	void PitchCraft(float val);
	void RollCraft(float val);
	void YawCraft(float val);

	void RearThrust(float val);
	void SideThrust(float val);
	void BottomThrust(float val);

	// ** Other Controls **

	// Toggle View
	void ToggleView();
	// Toggle Spot Light
	void ToggleSpotLight();

	// Fire Primary Weapon
	void firePrimary_pressed();
	void firePrimary_released();

	// Weapon Selection
	void weap_slot_1();
	void weap_slot_2();
	void weap_slot_3();
	void weap_slot_4();
	void weap_slot_5();
	void weap_slot_6();
	void weap_slot_7();
	void weap_slot_8();
	void weap_slot_9();
	void weap_slot_0();

	// How quickly forward speed changes
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float Acceleration;

	// How quickly pawn can steer 
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float TurnSpeed;

	// How quickly pawn returns to level
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float ReturnSpeed;

	// Max forward speed
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float MaxSpeed;

	// Min forward speed
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float MinSpeed;

	// Deceleraction Speed
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float Deceleration;

	// Axis Smoothing
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		float AxisSmoothing;

	// Third Or First Person Camera
	UPROPERTY(Category = SpaceRocksPawn, EditAnywhere)
		bool bIsThirdPerson;










private:


	// Add two sets of degrees together e.g. 350degs + 20degs = 10degs
	float AddDegrees(float startdeg, float adddeg);

	// Calculate the thrust factor for an axis from the yaw angle (0-360 degs)
	float CalcFactor_yaw(float craftangle, float offset_degs);

	// Calculate the thrust factor for an axis from the pitch angle (+/- 90 degs)
	float CalcFactor_pitch(float craftangle);

	// Calculate the thrust factor for an axis from the roll angle (+/- 180 degs)
	float CalcFactor_roll(float craftangle);

	// Calculate Thrust
	float CalcThrust(float InputVal, float factor, float CurrentAxisSpeed);

	// Projectile Fire/Placement/Direction
	FVector FireLocation;
	FRotator FireRotation;
	FVector FireDirection;
	FVector FireLocation_Mid_Left;
	FVector FireLocation_Mid_Right;
	FVector FireDirection_Mid_Left;
	FVector FireDirection_Mid_Right;
	FRotator FireRotation_Mid_Left;
	FRotator FireRotation_Mid_Right;
	FCollisionQueryParams CrossHair_TraceParams;
	FHitResult CrossHair_Hit;
	FVector LineTraceStart;
	FVector LineTraceEnd;

};
