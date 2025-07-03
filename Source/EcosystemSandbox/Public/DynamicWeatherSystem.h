#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Cloudy		UMETA(DisplayName = "Cloudy"),
	Overcast	UMETA(DisplayName = "Overcast"),
	LightRain	UMETA(DisplayName = "Light Rain"),
	HeavyRain	UMETA(DisplayName = "Heavy Rain"),
	Storm		UMETA(DisplayName = "Storm"),
	Fog			UMETA(DisplayName = "Fog"),
	Snow		UMETA(DisplayName = "Snow")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn"),
	Morning		UMETA(DisplayName = "Morning"),
	Noon		UMETA(DisplayName = "Noon"),
	Afternoon	UMETA(DisplayName = "Afternoon"),
	Dusk		UMETA(DisplayName = "Dusk"),
	Night		UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FWeatherSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeatherType WeatherType = EWeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudCoverage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindStrength = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SkyTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float LightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Temperature = 0.5f; // 0 = cold, 1 = hot
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FTimeOfDaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float CurrentHour = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float SunAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MoonColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float SunIntensity = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoonIntensity = 0.5f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherType, NewWeather, EWeatherType, PreviousWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay, float, CurrentHour);

UCLASS()
class ECOSYSTEMSANDBOX_API ADynamicWeatherSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	ADynamicWeatherSystem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UDirectionalLightComponent* SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkyLightComponent* SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* SkyDome;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* RainEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* SnowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* FogEffect;

	// Weather Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FWeatherSettings CurrentWeatherSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FTimeOfDaySettings TimeOfDaySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	bool bDynamicWeather = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherTransitionSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherChangeInterval = 120.0f; // 2 minutes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	bool bDynamicTimeOfDay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float DayDuration = 600.0f; // 10 minutes per day

	// Lumen Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
	bool bEnableLumenGI = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
	float LumenSceneDetailScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
	float LumenFinalGatherQuality = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
	float LumenReflectionQuality = 1.0f;

	// Material Instances for Dynamic Updates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	class UMaterialInterface* SkyDomeMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
	class UMaterialInstanceDynamic* DynamicSkyMaterial;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

public:
	// Public Functions
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EWeatherType NewWeather, float TransitionTime = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetTimeOfDay(float Hour);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	EWeatherType GetCurrentWeather() const { return CurrentWeatherSettings.WeatherType; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetCurrentHour() const { return TimeOfDaySettings.CurrentHour; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	ETimeOfDay GetTimeOfDayEnum() const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	FWeatherSettings GetCurrentWeatherSettings() const { return CurrentWeatherSettings; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void ForceWeatherUpdate();

	UFUNCTION(BlueprintCallable, Category = "Lumen")
	void UpdateLumenSettings();

	// Weather effects on ecosystem
	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetTemperatureEffect() const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetVisibilityModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetMovementModifier() const;

protected:
	// Internal Functions
	void InitializeComponents();
	void SetupLumenGlobalIllumination();
	void UpdateTimeOfDay(float DeltaTime);
	void UpdateWeatherEffects(float DeltaTime);
	void UpdateSunPosition();
	void UpdateSkyLighting();
	void UpdateWeatherParticles();
	void UpdateMaterialParameters();
	void TransitionToWeather(EWeatherType NewWeather);
	EWeatherType GenerateRandomWeather() const;

private:
	// Weather transition state
	FWeatherSettings TargetWeatherSettings;
	float WeatherTransitionTimer = 0.0f;
	float WeatherTransitionDuration = 5.0f;
	bool bWeatherTransitioning = false;

	// Time tracking
	float LastWeatherChangeTime = 0.0f;
	ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Noon;
	ETimeOfDay PreviousTimeOfDay = ETimeOfDay::Noon;

	// Cached references
	class ADirectionalLight* SunLightActor;
	class ASkyLight* SkyLightActor;
};
