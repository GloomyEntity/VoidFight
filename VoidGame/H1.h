#pragma once
#include <vector>

int YourX = 0;
int YourY = 0;

bool menu = true;
bool Play = false;
bool Pause = false;

const int BASE_HEIGHT = 1080;
const int BASE_WIDTH = BASE_HEIGHT * 16 / 9;
int HeightWin = BASE_HEIGHT;
int SizeWin = BASE_WIDTH;

//кнопки
char UP_key = 'W';
char DOWN_key = 'S';
char RIGHT_key = 'D';
char LEFT_key = 'A';

bool ShadowMove = true;
bool ShadowOst = true;
int Shadow_X_c = 0;
int Shadow_Y_c = 0;
int Shadow_x = 0;
int Shadow_y = 0;

