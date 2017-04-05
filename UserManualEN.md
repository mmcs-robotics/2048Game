# User manual
If you want just to try such a project, you'll need just a Visual Studio 2015 (perhaps it will work fine in other versions of VS).
## Instruction
1. This app requeres «2048» application with black numbers on colored tiles, for example: https://play.google.com/store/apps/details?id=com.digiplex.game.
2. Assemble an EV3 robot, capable of sliding tiles by two motors: motor A should perform slides up and down (one full rotation – one slide), motor B – left/right. Connect EV3 to PC, check out assigned COM-port (it's number needed to connect in app).
3. Connect a webcam, and place it above the screen.
4. Run «FinalDesktopProject» application, select your webcam, activate and calibrate it. Red squares should be located exactly on tiles. Use W A S D keys for centering cam field of view, Q and E – for changing unused border of image.
5. Select robot COM-port, connect and test it. Direction of motors rotations can be wrong, but it cannot be changed from running application, so you should find a place in code, where rotations is started, and fix it. Best way to change it – file «MainForm.cs», lines 145-148.
6. Now everything is ready for testing.

If you have any problems – check out YouTube video.
