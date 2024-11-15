// Fill out your copyright notice in the Description page of Project Settings.


#include "PolePawn.h"

// Sets default values
APolePawn::APolePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Cam = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));

	Cam->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APolePawn::BeginPlay()
{
	Super::BeginPlay();
	OnPause();
	Open_Connection();
	NetworkEnum = NetworkState::NS_WaitSendState;
	IsReceivStart = false;

}

void APolePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Close_Connection();
}

// Called every frame
void APolePawn::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	//
	//UE_LOG(LogTemp, Warning, TEXT("%d"),step);

	//if (NetworkEnum == NetworkState::NS_Reset) {
	//	UE_LOG(LogTemp, Warning, TEXT("Reset"));
	//	Reset_Env();
	//}
	//
	//if (NetworkEnum == NetworkState::NS_WaitSendState && IsReceivStart) {

	//	Send_State();
	//	UE_LOG(LogTemp, Warning, TEXT("NS_SendState"));
	//	NetworkEnum = NetworkState::NS_SendState;
	//	UE_LOG(LogTemp, Warning, TEXT("NS_WaitAction"));
	//	NetworkEnum = NetworkState::NS_WaitAction;
	//}

	//if (NetworkEnum == NetworkState::NS_WaitAction) {
	//	
	//	FVector currLocation = Base->GetComponentLocation();
	//	currLocation.X = currLocation.X + (CurrMotorSpeed * DeltaTime);
	//	Base->SetWorldLocation(currLocation);
	//	CurrMotorSpeed *= 0.9f;

	//}

	//Conduct_Connection();

	//if (NetworkEnum == NetworkState::NS_GetAction) {
	//	
	//	Send_Obs();
	//	UE_LOG(LogTemp, Warning, TEXT("NS_SendObs"));
	//	NetworkEnum = NetworkState::NS_SendObs;
	//	UE_LOG(LogTemp, Warning, TEXT("NS_WaitSendState"));
	//	NetworkEnum = NetworkState::NS_WaitSendState;
	//}
	//

	//step++;

	if (Input_ == 10) {
		UE_LOG(LogTemp, Warning, TEXT("resetting"));
		Reset_Env();
	}
	else if (Input_ == 11) {

	}
	else {
		// Apply Speed
		FVector currLocation = Base->GetComponentLocation();
		currLocation.X = currLocation.X + (CurrMotorSpeed * DeltaTime);
		Base->SetWorldLocation(currLocation);
		CurrMotorSpeed *= 0.9f;
	}

	// Conduct Connection - Gets Executed Once
	Conduct_Connection();

	if (Input_ != 10 && AppliedInput) {
		AppliedInput = false;
		//CustomTimeDilation = 0.0000001f;
		//Pole->SetSimulatePhysics(false);
		OnPause();

		int32 BytesSent = 0;
		TArray<uint8> Bytes;
		FRotator rotator = Pole->GetRelativeRotation();
		UE_LOG(LogTemp, Warning, TEXT("pitch: %f"), rotator.Pitch);
		uint8 rotation = rotator.Pitch;
		Bytes.Add(rotation);
		ConnectionSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
		
		uint8* Bytes2 = 0;
		//FString sRotation = FString::SanitizeFloat(rotator.Pitch);
		//sRotation += "/";
		//UE_LOG(LogTemp, Warning, TEXT("pitch: %s"), *sRotation);
		//StringToBytes(sRotation, Bytes2, BytesSent);
		//ConnectionSocket->Send(Bytes2, sizeof(Bytes2), BytesSent);
	}

	if (Input_ == 10) {
		Input_ = 11;
	}

	if (HasInput) {
		HasInput = false;
	}
}

//get action
void APolePawn::Conduct_Connection() {
	// Accept Connection
	if (WaitingForConnection) {
		TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool hasConnection = false;
		if (ListenSocket->HasPendingConnection(hasConnection) && hasConnection) {
			ConnectionSocket = ListenSocket->Accept(*RemoteAddress, TEXT("Connection"));
			WaitingForConnection = false;
			UE_LOG(LogTemp, Warning, TEXT("incoming connection"));

			// Start Recv Thread
			ClientConnectionFinishedFuture = Async(EAsyncExecution::LargeThreadPool, [&]() {
				UE_LOG(LogTemp, Warning, TEXT("recv thread started"));
				IsReceivStart = true;
				while (IsConnectionOpen) {
					uint32 size;
					TArray<uint8> ReceivedData;

					if (ConnectionSocket->HasPendingData(size)) {
						ReceivedData.Init(0, 10);
						int32 Read = 0;
						ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

						if (ReceivedData.Num() > 0)
						{
							Input_ = ReceivedData[0];
							HasInput = true;

							// Handle Input
							if (HasInput) {
								//CustomTimeDilation = 1.0f;
								//Pole->SetSimulatePhysics(true);
								OnResume();
								AppliedInput = true;

								if (Input_ == 10) {
									// nothing!
								}
								else {

									if (Input_ == 0)
										Input_ = -1;

									//get action
									UE_LOG(LogTemp, Warning, TEXT("%d"), Input_);
									UE_LOG(LogTemp, Warning, TEXT("NS_GetAction"));
									NetworkEnum = NetworkState::NS_GetAction;
									Move_XAxis(Input_);

								}

							}
						}
					}
				}
			});
		}
	}
}

void APolePawn::Reset_Env() {
	FVector currLocation = Base->GetComponentLocation();
	currLocation.X = 0;
	Base->SetWorldLocation(currLocation);

	CurrMotorSpeed = 0;

	FRotator rotator = Pole->GetRelativeRotation();
	rotator.Pitch = 0.0f;
	rotator.Roll = 0.0f;
	rotator.Yaw = 0.0f;
	Pole->SetRelativeRotation(rotator);

	FVector angularVel = FVector();
	Pole->SetPhysicsAngularVelocityInDegrees(angularVel);
	//Pole->SetPhysicsAngularVelocity(angularVel);
	Pole->SetAllPhysicsLinearVelocity(angularVel);
}

void APolePawn::Send_State()
{
}

void APolePawn::Send_Obs()
{
	OnPause();

	int32 BytesSent = 0;
	TArray<uint8> Bytes;
	FRotator rotator = Pole->GetRelativeRotation();
	UE_LOG(LogTemp, Warning, TEXT("pitch: %f"), rotator.Pitch);
	FString sRotation = FString::SanitizeFloat(rotator.Pitch);
	uint8 rotation = rotator.Pitch;
	Bytes.Add(rotation);
	ConnectionSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
}

// Called to bind functionality to input
void APolePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	InputComponent->BindAxis("X", this, &APolePawn::Move_XAxis);

	// InputComponent->BindAction("OC", IE_Pressed, this, &APolePawn::Open_Connection);
	// InputComponent->BindAction("CC", IE_Pressed, this, &APolePawn::Close_Connection);
}

void APolePawn::Move_XAxis(float AxisValue) {
	CurrMotorSpeed += AxisValue * MotorPower;
	CurrMotorSpeed = FMath::Clamp(CurrMotorSpeed, -MaxSpeed, MaxSpeed);
}

void APolePawn::Open_Connection() {
	if (!IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Openning Connection"));
		IsConnectionOpen = true;
		WaitingForConnection = true;

		FIPv4Address IPAddress;
		FIPv4Address::Parse(FString("127.0.0.1"), IPAddress);
		FIPv4Endpoint Endpoint(IPAddress, (uint16)7794);

		ListenSocket = FTcpSocketBuilder(TEXT("TcpSocket")).AsReusable();

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ListenSocket->Bind(*SocketSubsystem->CreateInternetAddr(Endpoint.Address.Value, Endpoint.Port));
		ListenSocket->Listen(1);
		UE_LOG(LogTemp, Warning, TEXT("Listening"));
	}
}

void APolePawn::Close_Connection() {
	if (IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Closing Connection"));
		IsConnectionOpen = false;

		ListenSocket->Close();
	}
}