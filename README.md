# 2048 Game
Solving 2048 game using LEGO EV3 robot. 
This repository contains two projects: 

1. The game itself, coming with AI solver for the puzzle; 
2. Controller for an EV3-based robot.

## Thanks and credits
1. Original game idea and design – Italian web developer [Gabriele Cirulli](https://en.wikipedia.org/wiki/2048_(video_game) "2048 Wikipedia page");
2. LEGO Mindstorms EV3 API for .NET – [Brian Peek, Clint Rutkas, Dan Fernandez](https://channel9.msdn.com/posts/LEGO-Mindstorms-EV3-API);
3. [AForge.NET library](http://www.aforgenet.com/ "Project page").

## Qt 5 game
2D visual game written in C++. It allows user to play the game, controlling the tiles with arrow keys. At any moment during the game you can turn on AI solver (based on alpha-beta pruning). Computer usually archives ~100000 points score. 

The project contains C++ solving unit, which is used in C# robot control project. The unit should be build as standalone DLL for using in robot control.

## C# LEGO EV3 robot control project
C# desktop application, using LEGO EV3 .NET API for robot control. AForge.NET library is used for capturing images from camera and processing them. Digit recognition is based on simple pattern matching. It is assumed that «2048» is running on smartphone or pad. EV3 robot performs slides by fully rotating its motors (2 motors used).

## Links
These projects were developed as part of «Intellectual systems» course in [Artificial intelligence and robotics lab](http://air.mmcs.sfedu.ru/), [Institute of mathematics, mechanics and computer science named after I.I. Vorovich](http://mmcs.sfedu.ru/), [Southern Federal University](http://sfedu.ru/).
