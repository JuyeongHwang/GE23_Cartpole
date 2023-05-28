// Fill out your copyright notice in the Description page of Project Settings.


#include "PoleActor.h"

// Sets default values
APoleActor::APoleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	//Goal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Goal"));
	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));

}

// Called when the game starts or when spawned
void APoleActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APoleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APoleActor::Reset_myEnv(int id) {

	FVector currLocation = Base->GetComponentLocation();
	currLocation.X = 200.0f;
	currLocation.Y = 300.0f*id;
	currLocation.Z = 0.0f;
	Base->SetWorldLocation(currLocation);

	//currLocation = Goal->GetComponentLocation();
	//currLocation.X = 200.0f + FMath::RandRange(-250, 250);
	//currLocation.Y = 300.0f * id;
	//currLocation.Z = 0.0f;
	//Goal->SetWorldLocation(currLocation);

	CurrMotorSpeed = 0;

	FRotator rotator = Pole->GetRelativeRotation();
	rotator.Pitch = 0.0f;
	rotator.Roll = 0.0f;
	rotator.Yaw = 0.0f;
	Pole->SetRelativeRotation(rotator);

	currLocation = Pole->GetComponentLocation();
	currLocation.X = 200.0f;
	currLocation.Y = 300.0f * id;
	currLocation.Z = 10.0f;
	Pole->SetWorldLocation(currLocation);


	FVector angularVel = FVector();
	Pole->SetPhysicsAngularVelocityInDegrees(angularVel);
	//Pole->SetPhysicsAngularVelocity(angularVel);
	Pole->SetAllPhysicsLinearVelocity(angularVel);
}

void APoleActor::Move_XAxis(int AxisValue)
{
	CurrMotorSpeed += AxisValue * MotorPower;
	CurrMotorSpeed = FMath::Clamp(CurrMotorSpeed, -MaxSpeed, MaxSpeed);
}

void APoleActor::ApplySpeed(float DeltaTime)
{
	FVector currLocation = Base->GetComponentLocation();
	currLocation.X = currLocation.X + (CurrMotorSpeed * DeltaTime);
	Base->SetWorldLocation(currLocation);
	CurrMotorSpeed *= 0.9f;
}
