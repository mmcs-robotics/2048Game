# 2048 Game
Solving 2048 game with LEGO EV3 robot. Here are two projects – just game itself with computer solver, and project for control EV3-based robot.
## Thanks and credits
1. Original game idea and design – Italian web developer [Gabriele Cirulli](https://en.wikipedia.org/wiki/2048_(video_game) "2048 Wikipedia page");
2. LEGO Mindstorms EV3 API for .NET – [Brian Peek, Clint Rutkas, Dan Fernandez](https://channel9.msdn.com/posts/LEGO-Mindstorms-EV3-API);
3. [AForge.NET library](http://www.aforgenet.com/ "Project page").

## Qt 5 game
Simple C++ visual project, game for human player. Tiles controlled by arrow keys, in any moment you can start AI solver (alpha-beta pruning). Computer usually archives ~100000 points score. This project contains C++ solving unit, which used in C# project for AI moving. This unit should be build as standalone DLL for using in robot control.
## C# LEGO EV3 robot control project
C# desktop application, with LEGO EV3 .NET API for robot control. Used AForge.NET library for camera image capturing and processing. Recognition is quite simple – just pattern matching. It is supposed that 2048 runs on smartphone or pad, and EV3 can perform slides by full rotation of motors (2 motors used).
## Links
This projects were developed as part of «Intellectual systems» course in [Artificial intelligence and robotics lab](http://air.mmcs.sfedu.ru/), [Institute of mathematics, mechanics and computer science named after I.I. Vorovich](http://mmcs.sfedu.ru/), [Southern Federal University](http://sfedu.ru/).
