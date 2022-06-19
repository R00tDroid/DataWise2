class ProjectConfiguration:
	def __init__(self):
		self.Name=""
		self.Type=""
		
		self.Defines=[]
		self.Libs=[]
		self.PostBuildCommands=[]
		self.BuildCommand=""
		self.CleanCommand=""
	
def DefaultConfigTemplate(config, platform, cpu, config_type):
	print("No configuration template defined for: " + config.Name)

class Project:
	def __init__(self, name):
		self.Name = name
		self.Dependencies = []
		self.SourcePathes=[]
		self.IncludePathes=[]
		self.ExportType=""
		
		self.Configs=[]
		self.TargetConfigs=[]
		self.ConfigTemplate = DefaultConfigTemplate
		
	def AddPlatform(self, Name, Arch):
		for config_type in self.Configs:
			for cpu_type in Arch:
				config = ProjectConfiguration()
				config.Name = Name.lower() + "_" + cpu_type.lower()
				config.Type = config_type.lower()
				self.ConfigTemplate(config, Name.lower(), cpu_type.lower(), config_type.lower())
				self.TargetConfigs.append(config)

class Solution:
	def __init__(self):
		self.Name=""
		self.Projects = []