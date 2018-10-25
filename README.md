# dll-injector
Cool little Windows dll injector written in C. Inject dll code into running processes to control them how you wish :)
It is best to compile your own executable before running >.> 
First make sure you have MinGW-w64 installed!!!!! If not download it from:

    https://sourceforge.net/projects/mingw-w64/
    
After, run:

    C:\Users\You\Somewhere>gcc -o inject main.c
    
That should create an exe file that you can run from the command line!!!! Run 'inject.exe' from the command line or double click and you should see the usage of the program.

    USAGE: inject.exe [DLL PATH] [-e (Program To Execute)] || [-p (Process Name)] || [-w (Window Title)]

    Press any key to continue . . . 
    
To run fully, go into the command line and run it how you wish and inject any process you want. Example:

    inject.exe C:\\Users\\You\\some-dll.dll -p notepad.exe
    
Any issues please write them in the issues (duh).
    
