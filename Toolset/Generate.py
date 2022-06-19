import os
import sys

sys.path.append(os.path.abspath("Scripts"))
from VSGen import ProjectGenerator
from VSGen import SolutionGenerator
from Project import CreateSolution

projects = {}

# Get solution and project definitions
solution = CreateSolution()

# Parse projects and configurations
for project in solution.Projects:
	generated_project = ProjectGenerator(project.Name)
	generated_project.SetProjectLocation("Projects")
	
	for source_dir in project.SourcePathes:
		generated_project.AddSourceDirectory(source_dir)
		
	for include_dir in project.SourcePathes:
		generated_project.AddIncludeDirectory(include_dir)
	
	for config in project.TargetConfigs:
		config_id = generated_project.AddConfig()
		generated_project.Config[config_id].Name = config.Name
		generated_project.Config[config_id].Type = config.Type
		generated_project.Config[config_id].Defines = config.Defines
		generated_project.Config[config_id].CmdBuild = config.BuildCommand
		generated_project.Config[config_id].CmdClean = config.CleanCommand
		generated_project.Config[config_id].build_commands = config.PostBuildCommands
		
	projects[project.Name] = generated_project
	
# Link project dependencies
for project in solution.Projects:
	for dependency in project.Dependencies:
		projects[project.Name].AddDependency(projects[dependency.Name])

# Generate project files		
for project_id in projects:
	projects[project_id].Generate()
	

# Generate solution
generated_solution = SolutionGenerator("Analytics Toolset")
for project_id in projects:
	generated_solution.AddProject(projects[project_id])
	
generated_solution.Generate()