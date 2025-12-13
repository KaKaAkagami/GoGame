#include "App.h"

//g++ -std=c++17 -I include src\*.cpp src\screens\*.cpp src\widgets\*.cpp -o GoGame.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

int main()
{
    App app;
    app.run();
    return 0;

}

