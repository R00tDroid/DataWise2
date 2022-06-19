import os, glob, uuid, configparser, hashlib

class ProjectConfiguration:
	def __init__(self):
		self.Name = "unnamed"
		self.Type = "Debug"
		self.CmdBuild = "echo \"CmdBuild not set\""
		self.CmdClean = "echo \"CmdClean not set\""
		self.Defines = []
		self.build_commands = []
		
	def AddPostBuildCommand(self, cmd):
		self.build_commands.append(cmd)

		
class UUIDStorage:
	def __init__(self):
		self.config = configparser.ConfigParser()
		if(os.path.isfile(".vs\\VSGen.ini")):
			self.config.read(".vs\\VSGen.ini")
		self.Save()
		
	def Contains(self, name):
		if "UUID" in self.config:
			if name in self.config["UUID"]:
				return True
			else:
				return False
		else:
			return False
		
	def Set(self, name, data):
		if (not "UUID" in self.config):
			self.config["UUID"] = {}
		self.config["UUID"][name] = data
		self.Save()
	
	def Get(self, name):
		return self.config["UUID"][name]
		
	def Save(self):
		if(not os.path.isdir(".vs")):
			os.makedirs(".vs")
		file = open(".vs\\VSGen.ini", "w+") 
		self.config.write(file)
		file.close()


class HASHStorage:
	def __init__(self):
		self.config = configparser.ConfigParser()
		if(os.path.isfile(".vs\\VSGen.ini")):
			self.config.read(".vs\\VSGen.ini")
		self.Save()
		
	def Contains(self, name):
		if "HASH" in self.config:
			if name in self.config["HASH"]:
				return True
			else:
				return False
		else:
			return False
		
	def Set(self, name, data):
		if (not "HASH" in self.config):
			self.config["HASH"] = {}
		self.config["HASH"][name] = data
		self.Save()
	
	def Get(self, name):
		return self.config["HASH"][name]
		
	def Save(self):
		if(not os.path.isdir(".vs")):
			os.makedirs(".vs")
		file = open(".vs\\VSGen.ini", "w+") 
		self.config.write(file)
		file.close()

		
def ValidatedCopy(name, temp_file, target_file):
	hasher = hashlib.md5()

	with open(temp_file, 'rb') as afile:
		buf = afile.read()
		hasher.update(buf)
	hash = hasher.hexdigest()
	
	hash_storage = HASHStorage()
	
	if(hash_storage.Contains(name) and os.path.isfile(target_file)):
		if(hash == hash_storage.Get(name)):
			os.remove(temp_file)
			return False			
	
	hash_storage.Set(name, hash)
	os.replace(temp_file, target_file)
	return True
		

def AddDirectoryTree(List, Path):
	temp_list = []
	item = Path
	
	while(not item == ""):
		temp_list.append(item)
		item = item=os.path.dirname(item)
	
	for i in reversed(temp_list):
		if (not i in List):
			List.append(i)
		
class ProjectGenerator:
	def __init__(self, name):
		self.Config = []
		self.source_locations = []
		self.header_locations = []
		self.dependencies = []
		self.output_location = ""
		self.name = name
		self.UUID = ""
		
	def AddSourceDirectory(self, directory):
		if(not os.path.isabs(directory)):
			directory = os.getcwd() + "\\" + directory
		self.source_locations.append(directory)
		
	def AddIncludeDirectory(self, directory):
		if(not os.path.isabs(directory)):
			directory = os.getcwd() + "\\" + directory
		self.header_locations.append(directory)
		
	def SetProjectLocation(self, directory):
		self.output_location = directory
		
	def AddConfig(self):
		self.Config.append(ProjectConfiguration())
		return len(self.Config) - 1
		
	def AddDependency(self, other):
		self.dependencies.append(other)
		
	def Generate(self):
	
		UUID = UUIDStorage()
		
		if(UUID.Contains(self.name)):
			self.UUID = UUID.Get(self.name)
		else:
			self.UUID = str(uuid.uuid4())
			UUID.Set(self.name, self.UUID)
	
		source_files = []
		header_files = []
		
		for directory in self.source_locations:
			for file in glob.glob(directory + "\\**\\*.cpp", recursive=True):
				source_files.append(file)
			for file in glob.glob(directory + "\\**\\*.c", recursive=True):
				source_files.append(file)
				
		for directory in self.header_locations:
			for file in glob.glob(directory + "\\**\\*.h", recursive=True):
				header_files.append(file)
		
		if(self.output_location == ""):
			self.output_location = os.getcwd()
			
		if(not os.path.isabs(self.output_location)):
			self.output_location = os.getcwd() + "\\" + self.output_location
		
		if(not os.path.isdir(self.output_location)):
			os.makedirs(self.output_location)
			
		filename = self.output_location + "\\" + self.name + ".vcxproj"
		filename_temp = self.output_location + "\\" + self.name + ".vcxproj.tmp"
		
		file = open(filename_temp, "w+") 
		file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
		file.write("<Project DefaultTargets=\"Build\" ToolsVersion=\"15.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n")
		
		## config definitions
		file.write("<ItemGroup Label=\"ProjectConfigurations\">\n")
		for config in self.Config:
			file.write("<ProjectConfiguration Include=\"" + config.Name + "_" + config.Type + "|Win32\">\n")
			file.write("<Configuration>" + config.Name + "_" + config.Type + "</Configuration>\n")
			file.write("<Platform>Win32</Platform>\n")
			file.write("</ProjectConfiguration>\n")
		file.write("</ItemGroup>\n")
		
		# Globals
		file.write("<PropertyGroup Label=\"Globals\">\n")
		file.write("<ProjectGuid>{" + self.UUID + "}</ProjectGuid>\n")
		file.write("<MinimumVisualStudioVersion>15.0</MinimumVisualStudioVersion>\n")
		file.write("<PlatformToolset>v141</PlatformToolset>\n")
		file.write("</PropertyGroup>\n")
		
		# config settings
		file.write("<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.Default.props\" />\n")
		for config in self.Config:
			file.write("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" + config.Name + "_" + config.Type + "|Win32'\" Label=\"Configuration\">\n")
			file.write("<ConfigurationType>Makefile</ConfigurationType>\n")
			file.write("<PlatformToolset>v141</PlatformToolset>\n")
			file.write("</PropertyGroup>\n")
			
			
		# config property sheets
		file.write("<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.props\" />\n")
		for config in self.Config:
			file.write("<ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" + config.Name + "_" + config.Type + "|Win32'\" Label=\"PropertySheets\">\n")
			file.write("<Import Project=\"$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n")
			file.write("</ImportGroup>\n")
			
		# config properties
		for config in self.Config:
		
			cmd = ""
			for command in config.build_commands:
				cmd += "\n" + command
		
			file.write("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)' == '" + config.Name + "_" + config.Type + "|Win32'\">\n")
			file.write("<IncludePath>$(VC_IncludePath);$(WindowsSDK_IncludePath)</IncludePath>\n")
			file.write("<NMakeBuildCommandLine>cd /d \"$(SolutionDir)\" &amp;&amp; " + config.CmdBuild + cmd + "</NMakeBuildCommandLine>\n")
			file.write("<NMakeCleanCommandLine>cd /d \"$(SolutionDir)\" &amp;&amp; " + config.CmdClean + "</NMakeCleanCommandLine>\n")
			file.write("<OutDir>$(SolutionDir)bin\\$(Configuration)</OutDir>\n")
			
			if(len(config.Defines) > 0):
				file.write("<NMakePreprocessorDefinitions>")
				
				for define in config.Defines:
					file.write(define+";")
				file.write("</NMakePreprocessorDefinitions>\n")
				
			file.write("</PropertyGroup>\n")

			
		file.write("<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.targets\" />\n")
		file.write("<ImportGroup Label=\"ExtensionSettings\" />\n")
		
		# Source file list
		file.write("<ItemGroup>\n")
		for source in source_files:
			file.write("<ClCompile Include=\"" + source + "\"/>\n")
		file.write("</ItemGroup>\n")
		
		# Header file list
		file.write("<ItemGroup>\n")
		for header in header_files:
			file.write("<ClInclude Include=\"" + header + "\"/>\n")
		file.write("</ItemGroup>\n")
		
		file.write("</Project>")
		
		file.close()
		
		if(ValidatedCopy(self.name + ".vcxproj", filename_temp, filename)):
			print("Created " + filename)
		else:
			print("Skipped " + filename)
		
		filename = self.output_location + "\\" + self.name + ".vcxproj.filters";
		filename_temp = self.output_location + "\\" + self.name + ".vcxproj.filters.tmp";
		
		file = open(filename_temp, "w+") 
		file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
		file.write("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n")
		
		filters = []
				
		for source_file in source_files:
			AddDirectoryTree(filters, os.path.relpath(os.path.dirname(source_file), os.getcwd()))
				
		for source_file in header_files:
			AddDirectoryTree(filters, os.path.relpath(os.path.dirname(source_file), os.getcwd()))
		
		# Filter definitions
		file.write("<ItemGroup>\n")
		
		UUID = UUIDStorage()
		
		for filter in filters:
			file.write("<Filter Include=\"" + filter + "\">\n")
			
			if(UUID.Contains(filter)):
				id = UUID.Get(filter)
			else:
				id = str(uuid.uuid4())
				UUID.Set(filter, id)
			
			file.write("<UniqueIdentifier>{" + str(id).upper() + "}</UniqueIdentifier>\n")
			file.write("</Filter>\n")
		
		file.write("</ItemGroup>\n")
		
		# File registry
		file.write("<ItemGroup>\n")
		
		for source_file in source_files:
			file.write("<ClCompile Include=\"" + source_file + "\">\n")
			file.write("<Filter>" + os.path.relpath(os.path.dirname(source_file), os.getcwd()) + "</Filter>\n")
			file.write("</ClCompile>\n")
			
		for source_file in header_files:
			file.write("<ClInclude Include=\"" + source_file + "\">\n")
			file.write("<Filter>" + os.path.relpath(os.path.dirname(source_file), os.getcwd()) + "</Filter>\n")
			file.write("</ClInclude>\n")
			
		file.write("</ItemGroup>\n")
		
		
		file.write("</Project>")
		file.close()
		
		if(ValidatedCopy(self.name + ".vcxproj.filters", filename_temp, filename)):
			print("Created " + filename)
		else:
			print("Skipped " + filename)
		
		
		# Debug information
		
		filename = self.output_location + "\\" + self.name + ".vcxproj.user";
		filename_temp = self.output_location + "\\" + self.name + ".vcxproj.user.tmp";
		
		file = open(filename_temp, "w+") 
		file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
		file.write("<Project ToolsVersion=\"15.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n")
		
		for config in self.Config:
			file.write("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)' == '" + config.Name + "_" + config.Type + "|Win32'\">\n")
			file.write("<LocalDebuggerCommand>$(SolutionDir)bin\\$(Configuration)\\" + self.name + ".exe</LocalDebuggerCommand>\n")
			file.write("<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n")
			file.write("</PropertyGroup>\n")
		
		file.write("</Project>")
		file.close()
		
		if(ValidatedCopy(self.name + ".vcxproj.user", filename_temp, filename)):
			print("Created " + filename)
		else:
			print("Skipped " + filename)
		
class SolutionGenerator:
	def __init__(self, name):
		self.output_location = ""
		self.name = name
		self.projects = []
		
	def AddProject(self, project):
		self.projects.append(project)
		
	def Generate(self):
		for project in self.projects:
			if(project.UUID == ""):
				print("Not all projects have been generated yet")
				return;
		
		if(self.output_location == ""):
			self.output_location = os.getcwd()
			
		if(not os.path.isabs(self.output_location)):
			self.output_location = os.getcwd() + "\\" + self.output_location
		
		if(not os.path.isdir(self.output_location)):
			os.makedirs(self.output_location)
			
		filename = self.output_location + "\\" + self.name + ".sln";
		filename_temp = self.output_location + "\\" + self.name + ".sln.tmp";
		
		file = open(filename_temp, "w+")
		file.write("Microsoft Visual Studio Solution File, Format Version 12.00\n")
		file.write("# Visual Studio 15\n")
		file.write("VisualStudioVersion = 15.0.28307.168\n")
		file.write("MinimumVisualStudioVersion = 10.0.40219.1\n")
		
		for project in self.projects:
			file.write("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" + project.name + "\", \"" + project.output_location + "\\" + project.name + ".vcxproj\", \"{" + project.UUID.upper() + "}\"\n")
			
			if(len(project.dependencies)>0):
				file.write("ProjectSection(ProjectDependencies) = postProject\n")
				
				for other in project.dependencies:
					file.write("{" + other.UUID.upper() + "} = {" + other.UUID.upper() + "}\n")
				
				file.write("EndProjectSection\n")
			
			file.write("EndProject\n")
		
		configs = []
		for project in self.projects:
			for config in project.Config:
				if(not config.Type + "|" + config.Name in configs):
					configs.append(config.Type + "|" + config.Name)
		
		file.write("Global\n")
		file.write("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n")
		for config in configs:
			file.write("\t\t" + config + " = " + config + "\n")
		file.write("\tEndGlobalSection\n")
		
		file.write("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n")
		for project in self.projects:
			for config in project.Config:
				file.write("\t\t{" + project.UUID.upper() + "}." + config.Type + "|" + config.Name + ".ActiveCfg = " + config.Name + "_" + config.Type + "|Win32\n")
				file.write("\t\t{" + project.UUID.upper() + "}." + config.Type + "|" + config.Name + ".Build.0 = " + config.Name + "_" + config.Type + "|Win32\n")
		file.write("\tEndGlobalSection\n")
		
		file.write("EndGlobal\n")
		file.close()
		
		if(ValidatedCopy(self.name + ".sln", filename_temp, filename)):
			print("Created " + filename)
		else:
			print("Skipped " + filename)
