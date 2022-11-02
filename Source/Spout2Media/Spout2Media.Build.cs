// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Spout2Media : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
	}

	public string GetPluginPath()
	{
		return Path.Combine(ModuleDirectory, "../../");
	}

	private string CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
	{
		string BinariesDir = Path.Combine(GetPluginPath(), "Binaries", Target.Platform.ToString());
		string Filename = Path.GetFileName(Filepath);

		//convert relative path 
		string FullBinariesDir = Path.GetFullPath(BinariesDir);

		if (!Directory.Exists(FullBinariesDir))
		{
			Directory.CreateDirectory(FullBinariesDir);
		}

		string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
		bool ValidFile = false;

		//File exists, check if they're the same
		if (File.Exists(FullExistingPath))
		{
			ValidFile = true;
		}

		//No valid existing file found, copy new dll
		if (!ValidFile)
		{
			File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
		}
		return FullExistingPath;
	}
	public Spout2Media(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ThirdPartyPath, "Spout/include")
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"MediaAssets",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RenderCore",
				"MediaUtils",
				"MediaIOCore",
				"RHI",
				"Projects",
				"D3D11RHI",
				"D3D12RHI",
				"Media",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		if ((Target.Platform == UnrealTargetPlatform.Win64))
		{
			string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "amd64" : "x86";
			PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.lib"));

			string pluginDLLPath = Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll");
			string binariesPath = CopyToProjectBinaries(pluginDLLPath, Target);
			System.Console.WriteLine("Using Spout DLL: " + binariesPath);
			RuntimeDependencies.Add(binariesPath);

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll"));
		}
	}
}
