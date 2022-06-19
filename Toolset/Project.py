import os, sys

sys.path.append(os.path.abspath("Scripts"))
from ProjectDef import Project
from ProjectDef import Solution

def CoreConfigTemplate(config, platform, cpu, config_type):
	config.BuildCommand = "call scons --module=core --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.CleanCommand = "call scons -c --module=core --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.Libs = ["Ws2_32"]
	config.Defines = ["_WIN32", "WIN32", "ANALYTICSTOOLSET_EXPORTS"]
	if(config_type.lower() == "release"):
		directory = platform + "_" + cpu
		config.PostBuildCommands.append("if not exist \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\" (mkdir \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\")")
		config.PostBuildCommands.append("if not exist \"$(SolutionDir)..\\Output\\Native\\DataWise\\Includes\" (mkdir \"$(SolutionDir)..\\Output\\Native\\DataWise\\Includes\")")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)bin\\$(Configuration)\\core.dll\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\"")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)bin\\$(Configuration)\\core.lib\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\"")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)Source\\Core\\RecordableSession.h\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Includes\"")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)Source\\Core\\PacketBuilder.h\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Includes\"")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)Source\\Core\\Export.h\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Includes\"")
	
def ServerConfigTemplate(config, platform, cpu, config_type):
	config.BuildCommand = "call scons --module=server --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.CleanCommand = "call scons -c --module=server --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.Libs = ["Ws2_32"]
	config.Defines = ["WIN32", "NOSSL", "NOLFS"]
	if(config_type.lower() == "release"):
		directory = platform + "_" + cpu
		config.PostBuildCommands.append("if not exist \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\" (mkdir \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\")")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)bin\\$(Configuration)\\server.exe\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" +  directory + "\"")
	
def CommandConfigTemplate(config, platform, cpu, config_type):
	config.BuildCommand = "call scons --module=dwc --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.CleanCommand = "call scons -c --module=dwc --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.Libs = ["Ws2_32"]
	config.Defines = ["WIN32"]
	if(config_type.lower() == "release"):
		directory = platform + "_" + cpu
		config.PostBuildCommands.append("if not exist \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\" (mkdir \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" + directory + "\")")
		config.PostBuildCommands.append("copy /b /y \"$(SolutionDir)bin\\$(Configuration)\\dwc.exe\" \"$(SolutionDir)..\\Output\\Native\\DataWise\\Bin\\" +  directory + "\"")
	
def TestConfigTemplate(config, platform, cpu, config_type):
	config.BuildCommand = "call scons --module=test --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.CleanCommand = "call scons -c --module=test --target=" + platform + " --cpu=" + cpu + " --runtime=" + config_type
	config.Defines = ["_WIN32"]
	config.Libs = ["bin\\" + platform.lower() + "_" + cpu.lower() + "_" + config_type.lower() + "\\Core"]

def CreateSolution():
		
		Core = Project("Core")
		Server = Project("Server")
		DWC = Project("DWC")
		Test = Project("Test")
		
		Core.ExportType = "library"
		Core.SourcePathes.append("Source\\Core")
		Core.IncludePathes.append("Source\\Core")
		Core.SourcePathes.append("Source\\Shared")
		Core.IncludePathes.append("Source\\Shared")
		Core.ConfigTemplate = CoreConfigTemplate
		Core.Configs = ["Debug", "Release"]
		Core.AddPlatform("windows", ["x64", "x86"])
		Core.AddPlatform("linux", ["x64", "x86", "arm"])
		Core.AddPlatform("android", ["x64", "arm", "armv7"])
		
		Server.ExportType = "executable"
		Server.SourcePathes.append("Source\\Server")
		Server.IncludePathes.append("Source\\Server")
		Server.SourcePathes.append("Source\\Shared")
		Server.IncludePathes.append("Source\\Shared")
		Server.ConfigTemplate = ServerConfigTemplate
		Server.Configs = ["Debug", "Release"]
		Server.AddPlatform("windows", ["x64", "x86"])
		#Core.AddPlatform("linux", ["x64", "x86", "arm"])
		
		DWC.ExportType = "executable"
		DWC.Dependencies.append(Server)
		DWC.SourcePathes.append("Source\\Command")
		DWC.IncludePathes.append("Source\\Command")
		DWC.SourcePathes.append("Source\\Shared")
		DWC.IncludePathes.append("Source\\Shared")
		DWC.SourcePathes.append("Source\\DevTools")
		DWC.IncludePathes.append("Source\\DevTools")
		DWC.ConfigTemplate = CommandConfigTemplate
		DWC.Configs = ["Debug", "Release"]
		DWC.AddPlatform("windows", ["x64", "x86"])
		#Core.AddPlatform("linux", ["x64", "x86", "arm"])
		
		Test.ExportType = "executable"
		Test.Dependencies.append(Core)
		Test.SourcePathes.append("Source\\Test")
		Test.IncludePathes.append("Source\\Test")
		Test.IncludePathes.append("Source\\Core")
		Test.ConfigTemplate = TestConfigTemplate
		Test.Configs = ["Debug", "Release"]
		Test.AddPlatform("windows", ["x64", "x86"])
		
		solution = Solution()
		solution.Projects.append(Core)
		solution.Projects.append(Server)
		solution.Projects.append(DWC)
		solution.Projects.append(Test)
		return solution