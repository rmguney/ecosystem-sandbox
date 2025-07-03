#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EcosystemSandboxGameMode.generated.h"

class AEcosystemManager;
class ANaniteEnvironment;
class ADynamicWeatherSystem;

UCLASS()
class ECOSYSTEMSANDBOX_API AEcosystemSandboxGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEcosystemSandboxGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
	TSubclassOf<AEcosystemManager> EcosystemManagerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
	TSubclassOf<ANaniteEnvironment> EnvironmentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
	TSubclassOf<ADynamicWeatherSystem> WeatherSystemClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ecosystem")
	AEcosystemManager* EcosystemManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ecosystem")
	ANaniteEnvironment* Environment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ecosystem")
	ADynamicWeatherSystem* WeatherSystem;

	// Simulation Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bAutoStartSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	float SimulationTimeScale = 1.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void StartSimulation();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void StopSimulation();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void ResetSimulation();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	AEcosystemManager* GetEcosystemManager() const { return EcosystemManager; }

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	ANaniteEnvironment* GetEnvironment() const { return Environment; }

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	ADynamicWeatherSystem* GetWeatherSystem() const { return WeatherSystem; }

protected:
	void SetupChaosPhysics();
	void InitializeEcosystemComponents();

private:
	bool bSimulationRunning = false;
};
