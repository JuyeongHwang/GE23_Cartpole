// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoleActor.generated.h"

UCLASS()
class GE_REINFORCEMENT_API APoleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APoleActor();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Reset_myEnv(int id);
	void Move_XAxis(int Input_);
	void ApplySpeed(float d);



	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMeshComponent* Base;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMeshComponent* Pole;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMeshComponent* Goal;

	UPROPERTY(EditAnywhere)
		float MaxSpeed;

	UPROPERTY(EditAnywhere)
		float XBoundary;

	UPROPERTY(EditAnywhere)
		float MotorPower;

	UPROPERTY(EditAnywhere)
		float CurrMotorSpeed;
};
