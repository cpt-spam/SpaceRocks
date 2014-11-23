// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SpaceRocks.h"
#include "SpaceRocksPawn.h"

ASpaceRocksPawn::ASpaceRocksPawn(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP)
{
	// Structure to hold one-time initialization - i.e. load all the Player's craft meshes etc.
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> ShieldMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("StaticMesh'/Game/SpaceRocks/StaticMeshes/PlayerCraft/UFO.UFO'"))
			, ShieldMesh(TEXT("StaticMesh'/Game/SpaceRocks/StaticMeshes/PlayerCraft/Shield.Shield'"))
		{
		}

	};
	static FConstructorStatics ConstructorStatics;
	

	// ** Let's Build the player's craft **

	// Firstly, the defence shield static mesh. This will normally not be visible (unless damage/crash/etc), but will form our root component.

	ShieldMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ShieldMesh00"));
	ShieldMesh->SetStaticMesh(ConstructorStatics.ShieldMesh.Get());
	RootComponent = ShieldMesh;
	// Turn on physics for static mesh - how cool is this! Craft will be subject to world physics (e.g. fall due to gravity etc)
	ShieldMesh->SetSimulatePhysics(false);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);



	// Next, create Next, the static mesh component for the Craft itself and attach to root

	PlaneMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	PlaneMesh->AttachTo(RootComponent);
	PlaneMesh->SetSimulatePhysics(false);	// Note we are turning off physics/collision for this mesh - Handled instead at Shield
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Now we create a spring arm component. This will be to attach a camera to for Third Person View
	SpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArm0"));
	SpringArm->AttachTo(PlaneMesh);	// Attach it to the plane mesh, so we follow its orientaion, not the root's
	//SpringArm->AttachTo(RootComponent);	// ** DEBUG ** 
	//SpringArm->TargetArmLength = 460.0f; // The camera follows at this distance behind the character	
	SpringArm->TargetArmLength = 1000.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f, 0.f, 160.f);
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 1.0f;
	SpringArm->bDoCollisionTest = false;




	// Now create Third Person camera component

	TP_Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("CameraTP0"));
	TP_Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);	// Attach to spring arm
	//TP_Camera->bUseControllerViewRotation = false; // Don't rotate camera with controller
	TP_Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	//TP_Camera->PostProcessSettings.AutoExposureMaxBrightness = 0.1;
	TP_Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	TP_Camera->PostProcessSettings.AutoExposureMinBrightness = 0.05;
	TP_Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	TP_Camera->PostProcessSettings.AutoExposureMaxBrightness = 0.05;

	// Now create First Person camera component 
	FP_Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("CameraFP0"));
	FP_Camera->AttachTo(PlaneMesh);	// Attach to Craft Mesh
	//FP_Camera->bUseControllerViewRotation = false; // Don't rotate camera with controller
	FP_Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	FP_Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	FP_Camera->PostProcessSettings.AutoExposureMinBrightness = 0.05;
	FP_Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	FP_Camera->PostProcessSettings.AutoExposureMaxBrightness = 0.05;

	// Now Add the spotlight 

	CraftSpotLight = PCIP.CreateDefaultSubobject<USpotLightComponent>(this, TEXT("CraftSpotLight0"));
	CraftSpotLight->Activate();
	CraftSpotLight->SetVisibility(true);
	CraftSpotLight->Intensity = 30000.f;
	CraftSpotLight->SetMobility(EComponentMobility::Movable);
	CraftSpotLight->AttachTo(PlaneMesh);
	CraftSpotLight->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	CraftSpotLight->AttenuationRadius = 5000.f;
	CraftSpotLight->SetCastShadows(true);


	// Enable Third Person Camera by default.
	bIsThirdPerson = true;
	FP_Camera->Deactivate();
	TP_Camera->Activate();



	// Set handling parameters
	Acceleration = 1000.f;
	TurnSpeed = 100.f;
	MaxSpeed = 4000.f;
	MinSpeed = -4000.f;
	CurrentXAxisSpeed = 0.f;
	CurrentZAxisSpeed = 0.f;
	CurrentYAxisSpeed = 0.f;
	Deceleration = 50.f;
	ReturnSpeed = 0.f;
	AxisSmoothing = 5.f;

	// Set Up Weapon Handling

	weapon = 0;
	lastfired = 0;
	primary_on = false;
	weap_cycle = 1;

	// -- Set up line trace to allow us to work out where the 2D crosshair is pointing in 3Dspace.
	CrossHair_TraceParams = FCollisionQueryParams(FName(TEXT("CrossHair__Trace")), true, this);
	CrossHair_TraceParams.bTraceComplex = true;
	CrossHair_TraceParams.bTraceAsyncScene = true;
	CrossHair_TraceParams.bReturnPhysicalMaterial = false;
	CrossHair_Hit = FHitResult(ForceInit);

	//Set Up Inventories

	//PlayerInv = ReconOneInventory();

	// Set Up Helpers

	//WeapInfo = ReconOneWeaponInfo();

	// Behaviour
	//ShieldLevel = MAX_SHIELD;
}

void ASpaceRocksPawn::OnConstruction(const FTransform& Transform)
{
	// Call Super class function
	Super::OnConstruction(Transform);

	// Switch to first person
	ToggleView();

	CurrentXAxisSpeed = 0.f;
	CurrentZAxisSpeed = 0.f;
	CurrentYAxisSpeed = 0.f;

	FRotator SpawnRotation = GetActorRotation();
	FVector SpawnForwardVector = GetActorForwardVector();

	FRotator SALRotation(0, 0, 0);
	SetActorRotation(SALRotation);
	PlaneMesh->AddLocalRotation(SpawnForwardVector.Rotation());

}

void ASpaceRocksPawn::Tick(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentXAxisSpeed * DeltaSeconds, CurrentYAxisSpeed * DeltaSeconds, CurrentZAxisSpeed * DeltaSeconds);

	// Move Craft's Root Component through X,Y and Z axis (with sweep so we stop when we collide with things)
	// Note that orientation/rotation of root component always remains fixed, but the craft's static mesh + camera do the rotation.
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame (For player's mesh and camera)
	FRotator DeltaRotation(0, 0, 0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;

	// Rotate Craft
	PlaneMesh->AddLocalRotation(DeltaRotation);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);

	// Are the fire button(s) pressed? If so, do something about it
	/*
	if (primary_on)
	{
		// Primary Fire

		// Just some test(ish) code to start with

		// Fire, if firerate not breached
		if (lastfired <= (this->GetWorld()->GetTimeSeconds() - WeapInfo.getFireRate(PlayerInv.weaponInventory[weapon])))
		{
			// ** TODO ** Can we make this all more efficient?

			// This big gets the Location an orientation of the player
			//FireLocation = ShieldMesh->RelativeLocation;
			FireLocation = RootComponent->RelativeLocation;
			FireRotation = PlaneMesh->RelativeRotation;

			// This gets the default direction the fire should head from the PlaneMesh forward vector.
			FireDirection = PlaneMesh->GetForwardVector();

			//Now we calculate various offsets to the player position, so we are not firing from the middle of the mesh



			// - Mid Left
			//FireLocation_Mid_Left = FireLocation;
			FireLocation_Mid_Left = FireLocation - PlaneMesh->GetRightVector() * 105.f;
			FireLocation_Mid_Left = FireLocation_Mid_Left + PlaneMesh->GetForwardVector() * 50.f;
			FireDirection_Mid_Left = FireDirection;
			FireRotation_Mid_Left = FireRotation;

			// - Mid Right
			//FireLocation_Mid_Right = FireLocation;
			FireLocation_Mid_Right = FireLocation + PlaneMesh->GetRightVector() * 105.f;
			FireLocation_Mid_Right = FireLocation_Mid_Right + PlaneMesh->GetForwardVector() * 50.f;
			FireDirection_Mid_Right = FireDirection;
			FireRotation_Mid_Right = FireRotation;

			// - Infront and centre

			FireLocation = FireLocation + FireDirection * 205.f;

			// This now changes the FireDirection by calculating the direction the fire should head to hit where the player's crosshair is.

			if (!bIsThirdPerson)
			{
				LineTraceStart = FireLocation;
				LineTraceEnd = LineTraceStart + FireDirection * 10000.f;
			}
			else
			{
				LineTraceStart = TP_Camera->GetComponentLocation();
				LineTraceEnd = LineTraceStart + TP_Camera->GetForwardVector() * 10000.f;
			}


			GetWorld()->LineTraceSingle(
				CrossHair_Hit,			 //result
				LineTraceStart,					//start
				LineTraceEnd,					//end
				ECC_Pawn,				//collision channel
				CrossHair_TraceParams
				);

			if (CrossHair_Hit.bBlockingHit)
			{
				FireDirection_Mid_Left = CrossHair_Hit.ImpactPoint - FireLocation_Mid_Left;

				FireRotation_Mid_Left = FireDirection_Mid_Left.Rotation();
				FireDirection_Mid_Right = CrossHair_Hit.ImpactPoint - FireLocation_Mid_Right;

				FireRotation_Mid_Right = FireDirection_Mid_Right.Rotation();
				CrossHair_Hit.bBlockingHit = false;	// why I have to reset this, I don't know? UE bug?
			}


			// Finally, fire the appropriate weapon
			if (PlayerInv.ammo[weapon] != 0)
			{

				AActor * spawned;

				if (PlayerInv.ammo[weapon] > 0) PlayerInv.ammo[weapon] --;

				switch (PlayerInv.weaponInventory[weapon])
				{
				case PHASEOID:
					spawned = GetWorld()->SpawnActor(AReconOneWP_phaseoid::StaticClass(), &FireLocation, &FireRotation);
					break;
				case PULSER:
					if (weap_cycle == 1 || weap_cycle == 3)
					{
						spawned = GetWorld()->SpawnActor(AReconOneWP_pulser::StaticClass(), &FireLocation_Mid_Left, &FireRotation_Mid_Left);
					}
					else
					{
						spawned = GetWorld()->SpawnActor(AReconOneWP_pulser::StaticClass(), &FireLocation_Mid_Right, &FireRotation_Mid_Right);
					}
					break;
				default:
					spawned = GetWorld()->SpawnActor(AReconOneWP_phaseoid::StaticClass(), &FireLocation, &FireRotation);
					break;
				}

				// Tag the projectile with the name of the Actor that spawned it.
				AReconOneWeaponProjectile* const TestProj = Cast<AReconOneWeaponProjectile>(spawned);
				if (TestProj)
				{
					TestProj->SpawnedBy = GetUniqueID();
				}

				lastfired = this->GetWorld()->GetTimeSeconds();
			}

			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetFName().ToString());
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("UniqueID = %d"), GetUniqueID()));



			// Increment weapon fire position cycle
			weap_cycle++;
			if (weap_cycle > 4)
			{
				weap_cycle = 1;
			}
		}
	}
	*/
}

void ASpaceRocksPawn::ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Force Actor rotation to be 0 - Only the Plane Mesh should rotate/

	FRotator SALRotation(0, 0, 0);
	SetActorRotation(SALRotation);


	// If we hit a projectile remove some shield

	/*
	AReconOneWeaponProjectile* const TestProj = Cast<AReconOneWeaponProjectile>(Other);
	if (TestProj)
	{

		ShieldLevel = ShieldLevel - TestProj->damage_delt;

	}
	else
	{*/
		// If it's not a projectile, bounce off (loosing some momentum in the process)
		// -- This bounce is a bit rubbish as I can't really work out the maths, but it sort of works well enough

		FVector ImpactAngle = Hit.Normal;

		ImpactAngle.X = (ImpactAngle.X * 2);
		if (ImpactAngle.X < 0) ImpactAngle.X = -ImpactAngle.X;
		ImpactAngle.X = ImpactAngle.X - 1;

		ImpactAngle.Y = (ImpactAngle.Y * 2);
		if (ImpactAngle.Y < 0) ImpactAngle.Y = -ImpactAngle.Y;
		ImpactAngle.Y = ImpactAngle.Y - 1;

		ImpactAngle.Z = (ImpactAngle.Z * 2);
		if (ImpactAngle.Z < 0) ImpactAngle.Z = -ImpactAngle.Z;
		ImpactAngle.Z = ImpactAngle.Z - 1;

		//CurrentXAxisSpeed = (CurrentXAxisSpeed * -ImpactAngle.X) / 2;
		//CurrentYAxisSpeed = (CurrentYAxisSpeed * -ImpactAngle.Y) / 2;
		//CurrentZAxisSpeed = (CurrentZAxisSpeed * -ImpactAngle.Z) / 2;
		CurrentXAxisSpeed = (CurrentXAxisSpeed * -ImpactAngle.X) / 1;
		CurrentYAxisSpeed = (CurrentYAxisSpeed * -ImpactAngle.Y) / 1;
		CurrentZAxisSpeed = (CurrentZAxisSpeed * -ImpactAngle.Z) / 1;


	//}
}

void ASpaceRocksPawn::ReceiveActorBeginOverlap(class AActor * Other)
{
	// We are overlaping another actor.

	// Is it a pickup? If so, do the needful

	/*

	int32 weap_slot = 0;

	AReconOnePickup* const TestPickup = Cast<AReconOnePickup>(Other);
	if (TestPickup && !TestPickup->IsPendingKill() && TestPickup->bIsActive)
	{
		if (!TestPickup->IsConsumable)
		{
			// It's a pickup. Let's pick it up and add it to the inventory (if there is room), then destroy
			if (!TestPickup->IsWeapon)
			{
				if (PlayerInv.AddToInv(TestPickup->PickupType))
				{
					TestPickup->OnPickedUp_Implementation();
				}
			}
			else
			{
				weap_slot = PlayerInv.AddToWeapInv(TestPickup->PickupType);
				if (weap_slot >= 0)
				{
					TestPickup->OnPickedUp_Implementation();
					weapon = weap_slot;
				}
			}
		}
		else
		{
			// It's some sort of consumable item. Do the needful

			if (TestPickup->PickupType == SHIELD_CHARGE)
			{
				// Top up shield

				ShieldLevel = ShieldLevel + 250;
				if (ShieldLevel > MAX_SHIELD) ShieldLevel = MAX_SHIELD;
				TestPickup->OnPickedUp_Implementation();
			}
			else
			{
				// Try adding as ammo pack
				if (PlayerInv.AddToAmmo(TestPickup->PickupType))
				{
					TestPickup->OnPickedUp_Implementation();
				}

			}
		}
	}
	*/
}


void ASpaceRocksPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// ** Handle Play Input **
	// The player's is controled through realistic(ish) spaceship thruster mechanics

	check(InputComponent);

	// ** Bind inputs to control functions **
	// Firstly, the orientation/axis thrusters

	InputComponent->BindAxis("PitchCraft", this, &ASpaceRocksPawn::PitchCraft);
	InputComponent->BindAxis("RollCraft", this, &ASpaceRocksPawn::RollCraft);
	InputComponent->BindAxis("YawCraft", this, &ASpaceRocksPawn::YawCraft);

	// Now, the directional thrusters

	InputComponent->BindAxis("ForwardCraft", this, &ASpaceRocksPawn::RearThrust);
	InputComponent->BindAxis("StrafeCraft", this, &ASpaceRocksPawn::SideThrust);
	InputComponent->BindAxis("UpCraft", this, &ASpaceRocksPawn::BottomThrust);

	// Misc Controls 

	InputComponent->BindAction("ToggleView", IE_Pressed, this, &ASpaceRocksPawn::ToggleView);
	InputComponent->BindAction("ToggleSpotLight", IE_Pressed, this, &ASpaceRocksPawn::ToggleSpotLight);

	// Weapon Control

	InputComponent->BindAction("PrimaryFire", IE_Pressed, this, &ASpaceRocksPawn::firePrimary_pressed);
	InputComponent->BindAction("PrimaryFire", IE_Released, this, &ASpaceRocksPawn::firePrimary_released);

	/*
	InputComponent->BindAction("Weap1", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_1);
	InputComponent->BindAction("Weap2", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_2);
	InputComponent->BindAction("Weap3", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_3);
	InputComponent->BindAction("Weap4", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_4);
	InputComponent->BindAction("Weap5", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_5);
	InputComponent->BindAction("Weap6", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_6);
	InputComponent->BindAction("Weap7", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_7);
	InputComponent->BindAction("Weap8", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_8);
	InputComponent->BindAction("Weap9", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_9);
	InputComponent->BindAction("Weap0", IE_Pressed, this, &ASpaceRocksPawn::weap_slot_0);
	*/
}

// Weapon Select
/*
void ASpaceRocksPawn::weap_slot_1()
{
	if (PlayerInv.weaponInventory[0] != EMPTY) weapon = 0;
}
void ASpaceRocksPawn::weap_slot_2()
{
	if (PlayerInv.weaponInventory[1] != EMPTY) weapon = 1;
}
void ASpaceRocksPawn::weap_slot_3()
{
	if (PlayerInv.weaponInventory[2] != EMPTY) weapon = 2;
}
void ASpaceRocksPawn::weap_slot_4()
{
	if (PlayerInv.weaponInventory[3] != EMPTY) weapon = 3;
}
void ASpaceRocksPawn::weap_slot_5()
{
	if (PlayerInv.weaponInventory[4] != EMPTY) weapon = 4;
}
void ASpaceRocksPawn::weap_slot_6()
{
	if (PlayerInv.weaponInventory[5] != EMPTY) weapon = 5;
}
void ASpaceRocksPawn::weap_slot_7()
{
	if (PlayerInv.weaponInventory[6] != EMPTY) weapon = 6;
}
void ASpaceRocksPawn::weap_slot_8()
{
	if (PlayerInv.weaponInventory[7] != EMPTY) weapon = 7;
}
void ASpaceRocksPawn::weap_slot_9()
{
	if (PlayerInv.weaponInventory[8] != EMPTY) weapon = 8;
}
void ASpaceRocksPawn::weap_slot_0()
{
	if (PlayerInv.weaponInventory[9] != EMPTY) weapon = 9;
}
*/

void ASpaceRocksPawn::PitchCraft(float val)
{
	// ** Player is firing Pitch Thrusters **
	// Target pitch speed is based in input

	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(val, 0.f);

	// If not turning, roll to reverse current roll value
	float TargetPitchSpeed = bHasInput ? (val * TurnSpeed * -1.f) : (PlaneMesh->RelativeRotation.Pitch * -ReturnSpeed);

	// Smoothly inerpolate to target pitch speed
	CurrentPitchSpeed = FMath::Clamp(FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), AxisSmoothing), MinSpeed, MaxSpeed);
}
void ASpaceRocksPawn::YawCraft(float val)
{
	// ** Player is firing Yaw Thrusters **
	// Target yaw speed is based in input	

	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(val, 0.f);

	// If not turning, don't do anything - We don't rest yaw like we do pitch and roll
	float TargetYawSpeed = bHasInput ? (val * TurnSpeed) : 0;

	// Smoothly interpolate yaw speed
	CurrentYawSpeed = FMath::Clamp(FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), AxisSmoothing), MinSpeed, MaxSpeed);
}
void ASpaceRocksPawn::RollCraft(float val)
{
	// ** Player is firing Roll Thrusters **
	// Target yaw speed is based in input	

	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(val, 0.f);

	// If not turning, roll to reverse current roll value
	float TargetRollSpeed = bHasInput ? (val * TurnSpeed) : (PlaneMesh->RelativeRotation.Roll * -ReturnSpeed);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::Clamp(FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), AxisSmoothing), MinSpeed, MaxSpeed);
}

void ASpaceRocksPawn::RearThrust(float val)
{

	// ** Player is Firing the Rear/Front Thrusters **
	// ** The orientation/rotation of the Root component is always fixed. **
	// ** So, speed up/slow down on each axis is based on the direction vector of where the player's craft mesh is pointing **

	float x_factor = 0.f;
	float y_factor = 0.f;
	float z_factor = 0.f;

	FVector Up = PlaneMesh->GetForwardVector();

	// ** Let's First calculate forward speed (aka X axis speed) **

	x_factor = Up.X;
	CurrentXAxisSpeed = CalcThrust(val, x_factor, CurrentXAxisSpeed);

	// ** Now for Y axis

	y_factor = Up.Y;
	CurrentYAxisSpeed = CalcThrust(val, y_factor, CurrentYAxisSpeed);

	// ** Now for Z axis speed **

	z_factor = Up.Z;
	CurrentZAxisSpeed = CalcThrust(val, z_factor, CurrentZAxisSpeed);


}

void ASpaceRocksPawn::SideThrust(float val)
{
	// ** Player is Firing the Side Thrusters **
	// ** The orientation/rotation of the Root component is always fixed. **
	// ** So, speed up/slow down on each axis is based on the direction vector of where the player's craft mesh is pointing **

	float x_factor = 0.f;
	float y_factor = 0.f;
	float z_factor = 0.f;

	FVector Up = PlaneMesh->GetRightVector();

	// ** Let's First calculate forward speed (aka X axis speed) **

	x_factor = Up.X;
	CurrentXAxisSpeed = CalcThrust(val, x_factor, CurrentXAxisSpeed);

	// ** Now for Y axis

	y_factor = Up.Y;
	CurrentYAxisSpeed = CalcThrust(val, y_factor, CurrentYAxisSpeed);

	// ** Now for Z axis speed **

	z_factor = Up.Z;
	CurrentZAxisSpeed = CalcThrust(val, z_factor, CurrentZAxisSpeed);
}
void ASpaceRocksPawn::BottomThrust(float val)
{

	// ** Player is Firing the Bottom/Top Thrusters **
	// ** The orientation/rotation of the Root component is always fixed. **
	// ** So, speed up/slow down on each axis is based on the direction vector of where the player's craft mesh is pointing **

	float x_factor = 0.f;
	float y_factor = 0.f;
	float z_factor = 0.f;

	FVector Up = PlaneMesh->GetUpVector();

	//float x = Up.X;
	//float y = Up.Y;
	//float z = Up.Z;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Pitch = %f, Roll = %f, Yaw = %f"), craftorientation.getPitchDegs(), craftorientation.getRollDegs(), craftorientation.getYawDegs()));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Pitch = %f, Roll = %f, Yaw = %f"), craftorientation.getPitchFactor(0), craftorientation.getRollFactor(0), craftorientation.getYawFactor(0)));


	// ** Let's First calculate forward speed (aka X axis speed) **

	x_factor = Up.X;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("vX = %f, vY = %f, vZ = %f"), x,y,z));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("X factor = %f"), x_factor));
	CurrentXAxisSpeed = CalcThrust(val, x_factor, CurrentXAxisSpeed);

	// ** Now for Y axis

	y_factor = Up.Y;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Y factor = %f"), y_factor));
	CurrentYAxisSpeed = CalcThrust(val, y_factor, CurrentYAxisSpeed);

	// ** Now for Z axis speed **

	z_factor = Up.Z;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Z factor = %f"), z_factor));
	CurrentZAxisSpeed = CalcThrust(val, z_factor, CurrentZAxisSpeed);

}


float ASpaceRocksPawn::CalcThrust(float InputVal, float factor, float CurrentAxisSpeed)
{

	float CurrentAcc = 0.f;
	float NewSpeed = 0.f;

	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(InputVal, 0.f);

	if (bHasInput)
	{
		CurrentAcc = (InputVal * Acceleration) * factor;
	}
	else
	{

		if (FMath::IsNegativeFloat(CurrentAxisSpeed))
		{
			CurrentAcc = 0.5f * Deceleration;
		}
		else
		{
			CurrentAcc = -0.5f * Deceleration;
		}
	}
	NewSpeed = CurrentAxisSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	return FMath::Clamp(NewSpeed, MinSpeed, MaxSpeed);

}

void ASpaceRocksPawn::ToggleView()
{
	// ** Toggle the players view between First and Third Person **


	if (bIsThirdPerson)
	{
		bIsThirdPerson = false;
		FP_Camera->Activate();
		TP_Camera->Deactivate();
		PlaneMesh->SetVisibility(false, false);
	}
	else
	{
		bIsThirdPerson = true;
		FP_Camera->Deactivate();
		TP_Camera->Activate();
		PlaneMesh->SetVisibility(true, false);
	}


}

void ASpaceRocksPawn::ToggleSpotLight()
{
	// ** Toggle the player's Spot Light **

	CraftSpotLight->ToggleVisibility();



}

void ASpaceRocksPawn::firePrimary_pressed()
{
	// ** Start Firing the primary weapon **

	primary_on = true;


}

void ASpaceRocksPawn::firePrimary_released()
{
	// ** Stop Firing the primary weapon **

	primary_on = false;


}


bool ASpaceRocksPawn::LookForInv(int32 PUtype)
{
	// Does the passed pickup type exist in the inventory?

	//return PlayerInv.LookForInv(PUtype);
	return 0;
}

