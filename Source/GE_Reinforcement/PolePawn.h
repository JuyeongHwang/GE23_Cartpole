// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Networking.h"
#include "Async/Async.h"
#include "Sockets.h"
#include "Camera/CameraComponent.h"

#include "PolePawn.generated.h"



UENUM(BlueprintType)
enum class NetworkState : uint8 {
	NS_WaitSendState        UMETA(DisplayName = "WaitSendState"),
	NS_SendState       UMETA(DisplayName = "SendState"),
	NS_WaitAction        UMETA(DisplayName = "WaitAction"),
	NS_GetAction        UMETA(DisplayName = "GetAction"),
	NS_SendObs        UMETA(DisplayName = "SendObs"),
	NS_Reset        UMETA(DisplayName = "Reset"),
};



UCLASS()
class GE_REINFORCEMENT_API APolePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APolePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool IsConnectionOpen = false;
	bool WaitingForConnection = false;
	bool IsReceivStart = false;
	bool HasInput = false;
	bool AppliedInput = false;
	int Input_ = 0;
	TFuture<void> ClientConnectionFinishedFuture;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		NetworkState NetworkEnum;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
		UStaticMeshComponent* Base;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMeshComponent* Pole;

	UPROPERTY(EditAnywhere)
		USceneComponent* PoleController;

	UPROPERTY(EditAnywhere)
		USceneComponent* Cam;

	UPROPERTY(EditAnywhere)
		USceneComponent* _BaseComponent;

	UPROPERTY(EditAnywhere)
		float MaxSpeed;

	UPROPERTY(EditAnywhere)
		float XBoundary;

	UPROPERTY(EditAnywhere)
		float MotorPower;

	UPROPERTY(EditAnywhere)
		float CurrMotorSpeed;

	UPROPERTY(EditAnywhere)
		AWorldSettings* WorldSettings;

	UFUNCTION(BlueprintImplementableEvent)
		void OnPause();

	UFUNCTION(BlueprintImplementableEvent)
		void OnResume();

	void Move_XAxis(float AxisValue);

	void Open_Connection();
	void Close_Connection();
	void Conduct_Connection();
	void Reset_Env();
	void Send_State();
	void Send_Obs();

	float Timer = 0;
	int step = 0;
	FSocket* ListenSocket;
	FSocket* ConnectionSocket;
};
