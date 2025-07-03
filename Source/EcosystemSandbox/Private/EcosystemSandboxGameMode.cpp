#include "EcosystemSandboxGameMode.h"
#include "EcosystemManager.h"
#include "NaniteEnvironment.h"
#include "DynamicWeatherSystem.h"
#include "Creature.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/WorldSettings.h"

AEcosystemSandboxGameMode::AEcosystemSandboxGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set default classes
	EcosystemManagerClass = AEcosystemManager::StaticClass();
	EnvironmentClass = ANaniteEnvironment::StaticClass();
	WeatherSystemClass = ADynamicWeatherSystem::StaticClass();
}

void AEcosystemSandboxGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetupChaosPhysics();
	InitializeEcosystemComponents();

	if (bAutoStartSimulation)
	{
		StartSimulation();
	}
}

void AEcosystemSandboxGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply time scale if simulation is running
	if (bSimulationRunning && GetWorld())
	{
		AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
		if (WorldSettings)
		{
			WorldSettings->SetTimeDilation(SimulationTimeScale);
		}
	}
}

void AEcosystemSandboxGameMode::StartSimulation()
{
	if (bSimulationRunning)
	{
		return;
	}

	bSimulationRunning = true;
	
	if (EcosystemManager)
	{
		EcosystemManager->InitializeEcosystem();
	}

	if (Environment)
	{
		Environment->InitializeEnvironment();
	}

	UE_LOG(LogTemp, Warning, TEXT("Ecosystem Simulation Started"));
}

void AEcosystemSandboxGameMode::StopSimulation()
{
	bSimulationRunning = false;

	// Reset time scale
	if (GetWorld())
	{
		AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
		if (WorldSettings)
		{
			WorldSettings->SetTimeDilation(1.0f);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Ecosystem Simulation Stopped"));
}

void AEcosystemSandboxGameMode::ResetSimulation()
{
	if (EcosystemManager)
	{
		EcosystemManager->ResetEcosystem();
	}

	if (Environment)
	{
		Environment->ResetEnvironment();
	}

	UE_LOG(LogTemp, Warning, TEXT("Ecosystem Simulation Reset"));
}

void AEcosystemSandboxGameMode::SetupChaosPhysics()
{
	// Enable Chaos Physics globally
	if (GetWorld())
	{
		AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
		if (WorldSettings)
		{
			// Set physics settings for optimal Chaos performance
			WorldSettings->bEnableWorldBoundsChecks = false;
			WorldSettings->bWorldGravitySet = true;
			WorldSettings->WorldGravityZ = -980.0f; // Standard gravity
		}
	}

	// Enable Lumen Global Illumination
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ReflectionMethod 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Lumen.Reflections.Allow 1"));
		GEngine->Exec(GetWorld(), TEXT("r.SkyLight.RealTimeReflectionCapture 1"));
		
		UE_LOG(LogTemp, Warning, TEXT("Lumen Global Illumination enabled in GameMode"));
	}
}

void AEcosystemSandboxGameMode::InitializeEcosystemComponents()
{
	if (!GetWorld())
	{
		return;
	}

	// Spawn Weather System
	if (WeatherSystemClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		WeatherSystem = GetWorld()->SpawnActor<ADynamicWeatherSystem>(
			WeatherSystemClass,
			FVector(0, 0, 1000),
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (WeatherSystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dynamic Weather System spawned successfully"));
		}
	}

	// Spawn Environment
	if (EnvironmentClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		Environment = GetWorld()->SpawnActor<ANaniteEnvironment>(
			EnvironmentClass, 
			FVector::ZeroVector, 
			FRotator::ZeroRotator, 
			SpawnParams
		);

		if (Environment)
		{
			UE_LOG(LogTemp, Warning, TEXT("Environment spawned successfully"));
		}
	}

	// Spawn Ecosystem Manager
	if (EcosystemManagerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		EcosystemManager = GetWorld()->SpawnActor<AEcosystemManager>(
			EcosystemManagerClass, 
			FVector(0, 0, 100), 
			FRotator::ZeroRotator, 
			SpawnParams
		);

		if (EcosystemManager)
		{
			// Set creature class for the ecosystem manager
			EcosystemManager->CreatureClass = ACreature::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("Ecosystem Manager spawned successfully"));
		}
	}
}
