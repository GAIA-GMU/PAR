// Fill out your copyright notice in the Description page of Project Settings.
using System.IO;
using System;
using UnrealBuildTool;

public class Tutorial9 : ModuleRules
{
    private string PARPath
    {
        get { return Path.GetFullPath(Path.Combine(Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)), "../../../../../../PAR/")); }
    }

	public Tutorial9(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        LoadPAR(Target);
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
		// if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
		// {
		//		if (UEBuildConfiguration.bCompileSteamOSS == true)
		//		{
		//			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		//		}
		// }
	}

    public bool LoadPAR(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "" : "d";
            string PythonString = (Target.Platform == UnrealTargetPlatform.Win64) ? "" : "_d";
            string ConnPath = (Target.Platform == UnrealTargetPlatform.Win64) ? "opt" : "debug";
            string LibrariesPath = Path.Combine(PARPath, "libs");

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "lwnet" + PlatformString + ".lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "database" + PlatformString + ".lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "agentProc" + PlatformString + ".lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Python27" + PythonString + ".lib"));
            string connector_path = Environment.GetEnvironmentVariable("CONNECTOR_ROOT");
            PublicAdditionalLibraries.Add(Path.Combine(connector_path,"lib", ConnPath, "mysqlcppconn.lib"));
        }

        if (isLibrarySupported)
        {
            // Include path
            //PublicIncludePaths.Add("C:\\Python27\\include");
            PublicIncludePaths.Add(Path.Combine(PARPath, "agentProc"));
            PublicIncludePaths.Add(Path.Combine(PARPath, "database"));
            PublicIncludePaths.Add(Path.Combine(PARPath, "lwnets"));

        }

        Definitions.Add(string.Format("WITH_PAR_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}
