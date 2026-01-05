# Argus

Argus is a singleplayer Real Time Strategy(RTS) game project made in Unreal Engine. This project originally started as a workspace for me to experiment with Unreal Engine in my free time as I joined a game development studio working primarily in Unreal Engine. I've also treated this project as an opportunity to implement systems and architectures that I've been interested in learning more about. On past games projects, I used Entity Component Systems(ECS) pretty heavily, but was not involved in the original architecting nor implementation. ECS is the backbone of Argus, and I tried to author Argus's ECS in a way that benefits from spatial and temporal locality, while also having utilities that may be more accessible to folks with an Object Oriented programming background. In order to make building features in Argus convenient, I also make heavy use of code generation through an Unreal Engine plugin I made called ArgusCodeGeneratorPlugin. The plugin is not the cleanest nor most performant code, (string manipulation in C++ is definitely not my strong suit), but has been instrumental in spinning up all the utility functionality around the ECS, especially the classes that are needed to interface with Unreal Engine's data asset serialization.



Over time, this project has morphed out of the realm of an experiment, and closer to something that can one day be playable. I'm going to continue building out the core technical functionality of Argus in this repository, but may one day make a private fork of the repository if I get around to content authoring and something closer to game design instead of just game programming. My hope is that this can one day be a central technical repository that RTS games built in Unreal Engine can fork off of.



Detailed documentation, examples, resources, and reasonings will be added over time to the [Argus Wiki](https://github.com/Karazaa/Argus/wiki). It is there where I will one day begin documenting the high level structure, program flow, and future plans. All short-medium term tasks are going to be tracked via [GitHub Issues](https://github.com/Karazaa/Argus/issues).



# How to Run Argus

Right now, Argus is based on Unreal Engine version 5.7.1. In order to run Argus you would either need to install Unreal version 5.7.1 via the Epic Games launcher or build Unreal 5.7.1 from source code. 



To get the Epic Games Launcher, [visit this site](https://www.unrealengine.com/en-US/download).



To build Unreal Engine from source, you will need to [join the Epic Games Organization on GitHub](https://github.com/EpicGames) in order to [clone their Unreal Engine repository](https://github.com/EpicGames/UnrealEngine). After cloning the proper version number of the engine and running Setup.bat, you will need to call GenerateProjectFiles.bat with the following flags.

**-projectfiles -project="{path to Argus.uproject file}" -game -engine -dotnet**



This will generate an Argus.sln that you can open in Visual Studio 2022, and then build **DebugGameEditor** from in order to run the editor locally. After running the editor, I'd suggest opening the **Sandbox** level and hitting play! From there you can get a very basic sense for what is currently implemented and poke around.

Argus relies on no bespoke engine changes, so having a vanilla version of Unreal Engine is fine! Feel free to contact Karazaa on GitHub if you have any questions or need a little assistance building ArgusEditor. 

