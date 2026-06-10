# SunsetCraft_V2

> Personal Project to make my own minecraft cause I need to continue practicing since I'm not a full developer.

![Status](https://img.shields.io/badge/status-prototype-orange)
![Language](https://img.shields.io/badge/C%2B%2B-20-blue)
![Build](https://img.shields.io/badge/build-CMake-informational)
![Graphics](https://img.shields.io/badge/graphics-OpenGL-success)

![Screenshot of the game](Ressources/ScreenShot/Game_10.06.26.png)

## Resume

> So basically I wanted to redo the generation of [SunsetCraft](https://github.com/SunvyWasTaken/SunsetCraft), but I wanted to keep the First version untouch.
> After watching a bit of the video [
Reinventing Minecraft world generation by Henrik Kniberg](https://youtu.be/ob3VwY4JyzE).
> I wanted to try to add some "Drama" to my world gen, and it turns out pretty well. So after that I also wanted to add Network to my game, so I need to make the network system before making the game that's another reason why I wanted to redo the project from scratch.

## Now

> For the moment I can start a world which is not replicated yet, but you can start another session to join it in local. Yeah... I didn't make the option yet, It wouldn't take to long.

## Build
> I use vcpkg for the Thirdparty follow the step from [here](https://learn.microsoft.com/fr-fr/vcpkg/get_started/get-started-vs?pivots=shell-cmd). 
> 
> For the build I use CMake.
> 
> You can also use CLion it will be easier now that is free for non-commercial use.

#### Otherwise, you can go with the following methode.
> - Clone vcpkg at the root of the project.
> ```cmd
> git clone https://github.com/microsoft/vcpkg.git
> ```
> - After the installation you can run vcpkg
> ```cmd
> cd vcpkg && bootstrap-vcpkg.bat
> ```
> - Return to the root of the project **!!Without closing the cmd window!!**
> ```cmd
> cd ..
> ```
> 
> ```cmd
> vcpkg install
> ```
> - Create a folder "Build" and run CMake in the folder "Build"
> ```cmd
> CMake build
> ```
> **(I strongly recommend you to use the methode from microsoft learn and NOT this one)** [here](https://learn.microsoft.com/fr-fr/vcpkg/get_started/get-started-vs?pivots=shell-cmd).
