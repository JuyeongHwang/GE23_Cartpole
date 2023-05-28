// Fill out your copyright notice in the Description page of Project Settings.


#include "PolePawnOrigin.h"

// Sets default values
APolePawnOrigin::APolePawnOrigin()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player1;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Cam = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));

	Cam->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void APolePawnOrigin::BeginPlay()
{
	Super::BeginPlay();
	Open_Connection();

	//CustomTimeDilation = 0.0000001f;
	//Pole->SetSimulatePhysics(false);
	OnPause();
}

void APolePawnOrigin::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Close_Connection();
}

// void APolePawnOrigin::OnPause() { }

// void APolePawnOrigin::OnResume() { }

// Called every frame
void APolePawnOrigin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	// actor list로부터 rotator, location 가져와서 각각 보내기.
	if (Input_ != 10 && AppliedInput) {
		AppliedInput = false;
		//CustomTimeDilation = 0.0000001f;
		//Pole->SetSimulatePhysics(false);
		OnPause();



		FRotator rotator = Pole->GetRelativeRotation();
		FString serialized = FString::SanitizeFloat(rotator.Pitch);
		UE_LOG(LogTemp, Warning, TEXT("pitch: %f"), rotator.Pitch);
		TCHAR* serializedChar = serialized.GetCharArray().GetData();
		int32 size = FCString::Strlen(serializedChar);
		int32 sent = 0;
		ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);

		FVector currLocation = Base->GetComponentLocation();
		serialized = FString::SanitizeFloat(currLocation.X);
		UE_LOG(LogTemp, Warning, TEXT("pitch: %f"), currLocation.X);
		serializedChar = serialized.GetCharArray().GetData();
		size = FCString::Strlen(serializedChar);
		sent = 0;
		ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	}

	if (Input_ == 10) {
		Input_ = 11;
	}

	if (HasInput) {
		HasInput = false;
	}
}

void APolePawnOrigin::Conduct_Connection() {
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

void APolePawnOrigin::Reset_Env() {
	FVector currLocation = Base->GetComponentLocation();
	currLocation.X = 900.0f;
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

// Called to bind functionality to input
void APolePawnOrigin::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	InputComponent->BindAxis("X", this, &APolePawnOrigin::Move_XAxis);

	InputComponent->BindAction("OC", IE_Pressed, this, &APolePawnOrigin::Open_Connection);
	InputComponent->BindAction("CC", IE_Pressed, this, &APolePawnOrigin::Close_Connection);
}

void APolePawnOrigin::Move_XAxis(float AxisValue) {
	CurrMotorSpeed += AxisValue * MotorPower;
	CurrMotorSpeed = FMath::Clamp(CurrMotorSpeed, -MaxSpeed, MaxSpeed);
}

void APolePawnOrigin::Open_Connection() {
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

void APolePawnOrigin::Close_Connection() {
	if (IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Closing Connection"));
		IsConnectionOpen = false;

		ListenSocket->Close();
	}
}