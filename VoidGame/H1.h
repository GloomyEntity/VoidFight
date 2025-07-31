#pragma once
#include <vector>
#include <string>

using namespace std;

bool AnimeC = true;

int NumServerIp = 2;
string SERVER_IP = "192.168.0.1";
int NumServerPort = 1;
unsigned short PORT = 62137;

const string SaveSetting = "SaveSetting.txt";
bool Fullscreen = false;
const string SaveAbilities_NE = "SaveAbilities_NE.txt";
const string SaveAbilities_A_E = "SaveAbilities_A_E.txt";
const string SaveAbilities_P_E = "SaveAbilities_P_E.txt";

int YourX = 0;
int YourY = 0;

bool menu = true;
bool Check_Weapon = false;
bool Setting = false;
bool Play = false;
bool Pause = false;
bool CannotConect = false;

const float BASE_HEIGHT = 1080;
const float BASE_WIDTH = BASE_HEIGHT * 16 / 9;
float HeightWin = BASE_HEIGHT;
float SizeWin = BASE_WIDTH;

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


vector <string> Abilities_NE;
vector <string> Abilities_A_E;
vector <string> Abilities_P_E;
int minusG_AList;
int minusG_BList;
int minusG_CList;