#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <windows.h>
using namespace sf;

View view;
using GameWindow = RenderWindow;
GameWindow* window;

View Cam(float x, float y) {
	view.setCenter(x, y);
	return view;
}

wstring utf8_to_wstring(const string& str) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
    wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}
Font ComicRu;
Text TextGame;
wostringstream  text;
void Write(int razmer,float X,float Y,string YourText) {
    TextGame.setCharacterSize(razmer);
    TextGame.setPosition(Vector2f(X, Y));
    text.str(L"");
    text << utf8_to_wstring(YourText);
    TextGame.setString(text.str());
    window->draw(TextGame);
}