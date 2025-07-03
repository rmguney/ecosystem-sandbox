#include "EcosystemSandbox.h"

#define LOCTEXT_NAMESPACE "FEcosystemSandboxModule"

void FEcosystemSandboxModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("EcosystemSandbox Module Started"));
}

void FEcosystemSandboxModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("EcosystemSandbox Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEcosystemSandboxModule, EcosystemSandbox)
