#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <locale>
#include <string>
#include <windows.h>
#include <mutex>
#include <fstream>
#include <codecvt>
#include <ctime>
#include <sstream>
#include <format>
#include "H1.h"
#include "Cam.h"
#pragma warning(disable : 4996)

using namespace std;
using namespace sf;

Event event;
Vector2f MousePos;

// Функции для масштабирования
float scaleX(float x) {
    return x * (SizeWin / BASE_WIDTH);
}

float scaleY(float y) {
    return y * (HeightWin / BASE_HEIGHT);
}

//кнопки
RectangleShape PlayB(Vector2f(600, 200)); // начать игру
RectangleShape ExitB(Vector2f(600, 200)); // выйти

RectangleShape EquipmentB(Vector2f(600, 200)); // экипировка
RectangleShape AbilityB(Vector2f(40, 40 * 16 / 9)); // кнопка способности

RectangleShape BackB(Vector2f(20, 20 * 16 / 9)); // назад

RectangleShape SettingB(Vector2f(20, 20 * 16 / 9)); //настройки
RectangleShape FullscreenB(Vector2f(15, 15 * 16 / 9)); // кнопка "Fullscreen"
RectangleShape ShadowMoveB(Vector2f(15, 15 * 16 / 9)); // кнопка "След игрока"
RectangleShape ShadowOstB(Vector2f(15, 15 * 16 / 9));  // кнопка "Остаточное изображение"

RectangleShape Player(Vector2f(20, 20 * 16 / 9));      // Игрок
RectangleShape PlayerEnemy(Vector2f(20, 20 * 16 / 9)); // Игрок Враг
RectangleShape PlayerShadow;                           // тень игрока
RectangleShape PlayerShadowOst(Vector2f(20, 20 * 16 / 9)); // тень игрока

RectangleShape Wall1(Vector2f(250 * 2, 10 * 16 / 9));
RectangleShape Wall2(Vector2f(10, 250 * 16 / 9 * 2));
RectangleShape Wall3(Vector2f(260 * 2 - 10, 10 * 16 / 9));
RectangleShape Wall4(Vector2f(10, 250 * 16 / 9 * 2));

RectangleShape Wall_Equipment(Vector2f(5, BASE_HEIGHT));

Packet SendPacket; // Для отправки (главный поток)
Packet ReceivePacket;
string YourMessage;
string ServerMessage;

Clock moveTimer;
float moveCooldown = 0.125f;
Clock ShadowTimer;
const float CanWatchShadow = 0.2f;

RectangleShape BlackBlockForPause(Vector2f(SizeWin, HeightWin));

atomic<bool> running(false);
mutex vectorMutex;

void Save_Setting() {
    ofstream saveSetting(SaveSetting);
    if (saveSetting) {
        saveSetting << Fullscreen << "\n"
                    << ShadowMove << "\n"
                    << ShadowOst << "\n";
    }
    saveSetting.close();
    ofstream save_Abilities_NE(SaveAbilities_NE, ios::binary);
    for (const auto& sizeA_NE : Abilities_NE) {
        size_t sizeAbilities_NE = sizeA_NE.size();
        save_Abilities_NE.write(reinterpret_cast<const char*>(&sizeAbilities_NE), sizeof(sizeAbilities_NE));  // Размер строки
        save_Abilities_NE.write(sizeA_NE.c_str(), sizeAbilities_NE);  // Сама строка
    }
    ofstream save_Abilities_P_E(SaveAbilities_P_E, ios::binary);
    for (const auto& sizeP_E : Abilities_P_E) {
        size_t sizeAbilitiesP_E = sizeP_E.size();
        save_Abilities_P_E.write(reinterpret_cast<const char*>(&sizeAbilitiesP_E), sizeof(sizeAbilitiesP_E));  // Размер строки
        save_Abilities_P_E.write(sizeP_E.c_str(), sizeAbilitiesP_E);  // Сама строка
    }
    ofstream save_Abilities_A_E(SaveAbilities_A_E, ios::binary);
    for (const auto& sizeA_E : Abilities_A_E) {
        size_t sizeAbilities_E = sizeA_E.size();
        save_Abilities_A_E.write(reinterpret_cast<const char*>(&sizeAbilities_E), sizeof(sizeAbilities_E));  // Размер строки
        save_Abilities_A_E.write(sizeA_E.c_str(), sizeAbilities_E);  // Сама строка
    }
}

void LOAD_Setting() {
    ifstream loadSetting(SaveSetting);
    if (loadSetting) {
        loadSetting >> Fullscreen
                    >> ShadowMove
                    >> ShadowOst;
    }
    loadSetting.close();
    ifstream load_SaveAbilities_NE(SaveAbilities_NE, ios::binary);
    while (load_SaveAbilities_NE) {
        size_t size_Abilities_NE;
        load_SaveAbilities_NE.read(reinterpret_cast<char*>(&size_Abilities_NE), sizeof(size_Abilities_NE));  // Читаем размер
        if (!load_SaveAbilities_NE) break;
        string str_size_Abilities_NE(size_Abilities_NE, '\0');
        load_SaveAbilities_NE.read(&str_size_Abilities_NE[0], size_Abilities_NE);  // Читаем строку
        Abilities_NE.push_back(str_size_Abilities_NE);
    }
    ifstream load_SaveAbilities_P_E(SaveAbilities_P_E, ios::binary);
    while (load_SaveAbilities_P_E) {
        size_t size_Abilities_P_E;
        load_SaveAbilities_P_E.read(reinterpret_cast<char*>(&size_Abilities_P_E), sizeof(size_Abilities_P_E));  // Читаем размер
        if (!load_SaveAbilities_P_E) break;
        string str_size_Abilities_P_E(size_Abilities_P_E, '\0');
        load_SaveAbilities_P_E.read(&str_size_Abilities_P_E[0], size_Abilities_P_E);  // Читаем строку
        Abilities_P_E.push_back(str_size_Abilities_P_E);
    }
    ifstream load_SaveAbilities_A_E(SaveAbilities_A_E, ios::binary);
    while (load_SaveAbilities_A_E) {
        size_t size_Abilities_A_E;
        load_SaveAbilities_A_E.read(reinterpret_cast<char*>(&size_Abilities_A_E), sizeof(size_Abilities_A_E));  // Читаем размер
        if (!load_SaveAbilities_A_E) break;
        string str_size_Abilities_A_E(size_Abilities_A_E, '\0');
        load_SaveAbilities_A_E.read(&str_size_Abilities_A_E[0], size_Abilities_A_E);  // Читаем строку
        Abilities_A_E.push_back(str_size_Abilities_A_E);
    }
}


Clock EscTimer;
const float EscCooldown = 0.5f;
void KeysClient() {
    while(true){
        Sleep(5);
            float ESCAPE_elapsed = EscTimer.getElapsedTime().asSeconds();
            bool CanEsc = (ESCAPE_elapsed >= EscCooldown);
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                if (Play == true) {
                    if (Pause == true) {
                        Pause = false;
                    }
                    else { Pause = true; }
                }
                else if (menu == true) {
                    if (Check_Weapon == false && Setting == false && CanEsc ) {
                        window->close();
                        exit(EXIT_SUCCESS);
                    }
                    else if (Setting == true) {
                        Setting = false;
                    }
                    else if (Check_Weapon == true) {
                        Check_Weapon = false;
                        minusG_AList = 0;
                        minusG_BList = 0;
                        minusG_CList = 0;
                    }
                }
                EscTimer.restart();
                keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
            }
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                if (AnimeC) {
                    AnimeC = false;
                }
            }
        }
    }

vector <int> XandYother;
int NumS = 1;
int NumW = 0;
string wordM;
void ReceiveMessages(TcpSocket& socket) {
    while (running) {
        if (socket.receive(ReceivePacket) == Socket::Done) {
            ReceivePacket >> ServerMessage;
            cout << ServerMessage << endl; 
            if (ServerMessage[0] == 'V') {
                NumS = 1;
                NumW = 0;
                while (NumS < ServerMessage.size()) {
                    if (ServerMessage[NumS] == ',') {
                        switch (NumW) {
                        case 0:
                            if (ShadowMove == true || ShadowOst == true) {
                                if (YourX != stoi(wordM)) {
                                    if (ShadowMove == true) {
                                        if (YourX < stoi(wordM)) {
                                            Shadow_x = 1;
                                        }
                                        else { Shadow_x = 2; }
                                    }
                                    if (ShadowOst == true) {
                                        Shadow_X_c = YourX;
                                    }
                                }
                            }                       
                            YourX = stoi(wordM);
                            break;
                        case 1:
                            if (ShadowMove == true) {
                                if (YourY != stoi(wordM)) {
                                    if (ShadowMove == true) {
                                        if (YourY > stoi(wordM)) {
                                            Shadow_y = 1;
                                        }
                                        else { Shadow_y = 2; }
                                    }
                                    if (ShadowOst == true) {
                                        Shadow_Y_c = YourY;                                       
                                    }
                                }
                            }
                            YourY = stoi(wordM);
                            break;
                        }
                        wordM = "\0";
                        NumW = NumW + 1;
                    }
                    else {
                        wordM = wordM + ServerMessage[NumS];
                    }
                    NumS = NumS + 1;
                }
            }
            if (ServerMessage[0] == 'A') {
                vector<int> newData;
                NumS = 1;
                while (NumS < ServerMessage.size()) {
                    if (ServerMessage[NumS] == ',') {
                        newData.push_back(stoi(wordM));
                        wordM = "\0";
                    }
                    else {
                        wordM = wordM + ServerMessage[NumS];
                    }
                    NumS = NumS + 1;
                }
                // Блокируем и обновляем основной вектор
                lock_guard<mutex> lock(vectorMutex);
                XandYother = move(newData);  // Перемещаем данные
            }
            ReceivePacket.clear();
            ServerMessage = "\0";
        }
    }
}

thread receiverThread;

void SizeWindow() {
    while (true) {
        Sleep(10);
            if (window->getSize().x != SizeWin || window->getSize().y != HeightWin) {
                if (window->getSize().x != SizeWin) {
                    if (window->getSize().x >= 400 / 9 * 16) {
                        SizeWin = window->getSize().x;
                    }
                    else {
                        window->setSize(Vector2u(400 / 9 * 16, window->getSize().y));
                    }
                }
                if (window->getSize().y != HeightWin) {
                    if (window->getSize().y >= 400) {
                        HeightWin = window->getSize().y;
                    }
                    else {
                        window->setSize(Vector2u(window->getSize().x, 400));
                    }
                }

                Cam((SizeWin) / 2, (HeightWin) / 2);
                window->setView(view);

                // Установка позиций стен
                Wall1.setPosition(Vector2f(-100 * 2, -100 * 16 / 9 * 2));
                Wall2.setPosition(Vector2f(150 * 2, -100 * 16 / 9 * 2));
                Wall3.setPosition(Vector2f(-100 * 2, 150 * 16 / 9 * 2));
                Wall4.setPosition(Vector2f(-100 * 2, -100 * 16 / 9 * 2));

                // Установка размеров объектов
                Wall1.setSize(Vector2f(250 * 2, 10 * 16 / 9));
                Wall2.setSize(Vector2f(10, 250 * 16 / 9 * 2));
                Wall3.setSize(Vector2f(260 * 2 - 10, 10 * 16 / 9));
                Wall4.setSize(Vector2f(10, 250 * 16 / 9 * 2));

                if (find(Abilities_P_E.begin(), Abilities_P_E.end(), "PLittle") == Abilities_P_E.end()) {
                    Player.setSize(Vector2f(20, 20 * 16 / 9));
                }
                else{ Player.setSize(Vector2f(10, 10 * 16 / 9)); }
                PlayerShadowOst.setSize(Vector2f(20, 20 * 16 / 9));
                PlayerEnemy.setSize(Vector2f(20, 20 * 16 / 9));

                // Установка размеров кнопок
                PlayB.setSize(Vector2f(600, 200));
                EquipmentB.setSize(Vector2f(600, 200));
                ExitB.setSize(Vector2f(600, 200));
                BackB.setSize(Vector2f(20, 20 * 16 / 9));
                SettingB.setSize(Vector2f(20, 20 * 16 / 9));
                FullscreenB.setSize(Vector2f(15, 15 * 16 / 9));
                ShadowMoveB.setSize(Vector2f(15, 15 * 16 / 9));
                ShadowOstB.setSize(Vector2f(15, 15 * 16 / 9));
                Wall_Equipment.setSize(Vector2f(5, BASE_HEIGHT));

                // Установка позиций элементов интерфейса
                BackB.setPosition(Vector2f(view.getCenter().x - 500, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
                Wall_Equipment.setPosition(Vector2f(view.getCenter().x + 100, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
                FullscreenB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 400));
                ShadowMoveB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 370));
                ShadowOstB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 340));
                SettingB.setPosition(Vector2f(view.getCenter().x - 500, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
                PlayB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - BASE_HEIGHT / 2 + 100));
                EquipmentB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 140));
                ExitB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y + BASE_HEIGHT / 2 - 380));
            }
        }
    }

Texture TextureAnime;
RectangleShape GameAnime(Vector2f(SizeWin, HeightWin));
void Anime(string name, int col, int Xpos, int Ypos, int Size, int Height) {
    AnimeC = true;
    for (int x = 1; x <= col;x++) {
        if (AnimeC) {
            window->clear();
            GameAnime.setPosition(Vector2f(Xpos, Ypos));
            GameAnime.setSize(Vector2f(Size, Height));
            TextureAnime.loadFromFile(name + to_string(x) + ".png");
            GameAnime.setTexture(&TextureAnime);
            window->draw(GameAnime);
            window->display();
            if (name == "Image/Intro/Intro") {
                if (x == 1 || x == 10) {
                    Sleep(1000);
                }
                else { Sleep(200); }
            }
            else { Sleep(200); }
        }
    }
    if (col > 1) {
        window->clear();
    }
}


int main() {
    setlocale(LC_ALL, "RU");
    ComicRu.loadFromFile("v_CCWildWordsLower_v1.35.ttf");
    TextGame.setFont(ComicRu);

    LOAD_Setting();
    bool WasFullscreen;
    if (Fullscreen == false) {
         window = new GameWindow(VideoMode(SizeWin, HeightWin), L"VoidFight", Style::Default);
         WasFullscreen = false;
    }
    else {
    window = new GameWindow(VideoMode(SizeWin, HeightWin), L"VoidFight", Style::Fullscreen);
    WasFullscreen = true;
    }

    Image icon;
    icon.loadFromFile("Image/Icon.png");
    window->setIcon(32, 32, icon.getPixelsPtr());

    switch (NumServerIp) {
    case 1:SERVER_IP = "192.168.0.10";
        break;
    case 2:SERVER_IP = "home.suboot.ru";
        break;
    }
    switch (NumServerPort) {
    case 1:PORT = 62137;
        break;
    }

    std::cout << "СеКС" << SERVER_IP << "|| " << PORT << endl;
    thread Keys_C(KeysClient);
    Keys_C.detach();
    thread Size(SizeWindow);
    Size.detach();

    Cam(0, 0);
    window->setView(view);
    GameAnime.setFillColor(Color::White);
    Anime("Image/Intro/Intro", 10, view.getCenter().x - (view.getSize().x / 2), view.getCenter().y - (float(view.getSize().y / 1.2)), (SizeWin * 9 / 16), (HeightWin * 16 / 9));
    AnimeC = false;


    Wall1.setPosition(Vector2f(-100 * 2, -100 * 16 / 9 * 2));
    Wall2.setPosition(Vector2f(150 * 2, -100 * 16 / 9 * 2));
    Wall3.setPosition(Vector2f(-100 * 2, 150 * 16 / 9 * 2));
    Wall4.setPosition(Vector2f(-100 * 2, -100 * 16 / 9 * 2));

    Cam((SizeWin) / 2, (HeightWin) / 2);
    window->setView(view);
    BackB.setPosition(Vector2f(view.getCenter().x - 500, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
    Wall_Equipment.setPosition(Vector2f(view.getCenter().x + 100, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
    BackB.setPosition(Vector2f(view.getCenter().x - 500, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
    FullscreenB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 400));
    ShadowMoveB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 370));
    ShadowOstB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 340));
    SettingB.setPosition(Vector2f(view.getCenter().x - 500, view.getCenter().y - (BASE_HEIGHT / 2 - 40 * (16 / 9))));
    PlayB.setPosition(Vector2f(view.getCenter().x - (300), view.getCenter().y - (BASE_HEIGHT / 2) + 100));
    EquipmentB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 140));
    ExitB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y + (BASE_HEIGHT / 2) - 380));

    TcpSocket socket;

    // Кнопка "Играть"
    PlayB.setFillColor(Color::White);
    Texture PlayBT;
    PlayBT.loadFromFile("Image/button/Button_Play.png");
    PlayB.setTexture(&PlayBT);

    // Кнопка "Выход"
    ExitB.setFillColor(Color::White);
    Texture ExitBT;
    ExitBT.loadFromFile("Image/button/Button_Exit.png");
    ExitB.setTexture(&ExitBT);

    // Кнопка "экипировка"
    EquipmentB.setFillColor(Color::White);
    Texture EquipmentBT;
    EquipmentBT.loadFromFile("Image/button/Button_Weapоn.png");
    EquipmentB.setTexture(&EquipmentBT);

    // Кнопка "назад"
    BackB.setFillColor(Color::White);
    Texture BackBT;
    BackBT.loadFromFile("Image/button/back_b.png");
    BackB.setTexture(&BackBT);

    // Кнопка "настройки"
    SettingB.setFillColor(Color::White);
    Texture SettingBT;
    SettingBT.loadFromFile("Image/button/SettingB.png");
    SettingB.setTexture(&SettingBT);

    // Кнопка в настройках,классическая
    Texture ForSettingBT1;
    ForSettingBT1.loadFromFile("Image/button/Classic_b_1_Setting.png");
    Texture ForSettingBT2;
    ForSettingBT2.loadFromFile("Image/button/Classic_b_2_Setting.png");

    Texture AbilityBT;

    Player.setFillColor(Color::White);
    PlayerShadow.setFillColor(Color::White);

    BlackBlockForPause.setFillColor(Color(0, 0, 0, 100));
    PlayerShadow.setFillColor(Color(128, 128, 128, 255));
    PlayerShadowOst.setFillColor(Color(128, 128, 128, 255));

    while (true) {
        while (window->pollEvent(event))
        {
            if (event.type == Event::Closed) {
                running = false;
                window->close();
            }     
            if (Check_Weapon) {
                if (MousePos.x <= view.getCenter().x - 360 && MousePos.x >= view.getCenter().x - 400) {
                    if (event.type == Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                        if (event.mouseWheelScroll.delta > 0) {
                            minusG_AList = minusG_AList + 20;
                        }
                        else if (event.mouseWheelScroll.delta < 0) {
                            minusG_AList = minusG_AList - 20;
                        }
                    }
                }
                if (MousePos.x <= view.getCenter().x - 260 && MousePos.x >= view.getCenter().x - 300) {
                    if (event.type == Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                        if (event.mouseWheelScroll.delta > 0) {
                            minusG_BList = minusG_BList + 20;
                        }
                        else if (event.mouseWheelScroll.delta < 0) {
                            minusG_BList = minusG_BList - 20;
                        }
                    }
                }
                if (MousePos.x <= view.getCenter().x - 160 && MousePos.x >= view.getCenter().x - 200) {
                    if (event.type == Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                        if (event.mouseWheelScroll.delta > 0) {
                            minusG_CList = minusG_CList + 20;
                        }
                        else if (event.mouseWheelScroll.delta < 0) {
                            minusG_CList = minusG_CList - 20;
                        }
                    }
                }
            }
        }
        if (menu == true) {  
            window->clear();
            MousePos = window->mapPixelToCoords(Mouse::getPosition(*window));
            Cam((SizeWin) / 2, (HeightWin) / 2);
            PlayB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 440));
            ExitB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y + 240));
            window->setView(view);
            if (Check_Weapon == false && Setting == false) {              
                window->draw(PlayB);
                window->draw(ExitB);
                window->draw(EquipmentB);
                window->draw(SettingB);
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    if (PlayB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        Play = true;
                        menu = false;
                    }
                    if (ExitB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        window->close();
                        exit(EXIT_SUCCESS);
                    }
                    if (EquipmentB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        Check_Weapon = true;
                    }
                    if (SettingB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        Setting = true;
                    }
                }
                if (CannotConect == true) {
                    BlackBlockForPause.setPosition(Vector2f(view.getCenter().x - SizeWin / 2, view.getCenter().y - HeightWin / 2));
                    window->draw(BlackBlockForPause);
                    Write(20, view.getCenter().x - 180, view.getCenter().y - 20, "Ошибка подключения\n(нажмите пробел или левую кнопку мыши)");
                    if (Mouse::isButtonPressed(Mouse::Left) || GetAsyncKeyState(VK_SPACE)) {
                        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        CannotConect = false;
                    }
                }
            } 
            else if(Check_Weapon == true){
                window->draw(Wall_Equipment);
                window->draw(BackB);
                Write(10, view.getCenter().x - 420, view.getCenter().y - 490, "Список \n способностей");
                Write(10, view.getCenter().x - 320, view.getCenter().y - 490, "Активные \n способности");
                Write(10, view.getCenter().x - 220, view.getCenter().y - 490, "Пассивные \n способности");
                if (!Abilities_NE.empty()) {
                    for (int x = 0; x < Abilities_NE.size();x++) {
                        AbilityB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 440 + (x * 100) + minusG_AList));
                        AbilityBT.loadFromFile("Image/abilities/" + Abilities_NE[x] + ".png");
                        AbilityB.setTexture(&AbilityBT);
                        window->draw(AbilityB);
                    }
                }
                if (!Abilities_A_E.empty()) {
                    for (int x = 0; x < Abilities_A_E.size();x++) {
                        AbilityB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 440 + (x * 100) + minusG_BList));
                        AbilityBT.loadFromFile("Image/abilities/" + Abilities_A_E[x] + ".png");
                        AbilityB.setTexture(&AbilityBT);
                        window->draw(AbilityB);
                    }
                }
                if (!Abilities_P_E.empty()) {
                    for (int x = 0; x < Abilities_P_E.size();x++) {
                        AbilityB.setPosition(Vector2f(view.getCenter().x - 200, view.getCenter().y - 440 + (x * 100) + minusG_CList));
                        AbilityBT.loadFromFile("Image/abilities/" + Abilities_P_E[x] + ".png");
                        AbilityB.setTexture(&AbilityBT);
                        window->draw(AbilityB);
                    }
                }
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    if (BackB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        minusG_AList = 0;
                        minusG_BList = 0;
                        minusG_CList = 0;
                        Check_Weapon = false;
                    }
                    if (!Abilities_NE.empty()) {
                        for (int x = 0; x < Abilities_NE.size();x++) {
                            AbilityB.setPosition(Vector2f(view.getCenter().x - 400, view.getCenter().y - 440 + (x * 100) + minusG_AList));
                            if (AbilityB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                                if (Abilities_NE[x][0] == 'A') {
                                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                                    Abilities_A_E.push_back(Abilities_NE[x]);
                                    Abilities_NE.erase(find(Abilities_NE.begin(), Abilities_NE.end(), Abilities_NE[x]));
                                }
                                else if (Abilities_NE[x][0] == 'P') {
                                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                                    Abilities_P_E.push_back(Abilities_NE[x]);
                                    Abilities_NE.erase(find(Abilities_NE.begin(), Abilities_NE.end(), Abilities_NE[x]));
                                }
                                Save_Setting();
                            }
                        }
                    }
                    if (!Abilities_A_E.empty()) {
                        for (int x = 0; x < Abilities_A_E.size();x++) {
                            AbilityB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 440 + (x * 100) + minusG_BList));
                            if (AbilityB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                                if (Abilities_A_E[x][0] == 'A') {
                                    Abilities_NE.push_back(Abilities_A_E[x]);
                                    Abilities_A_E.erase(find(Abilities_A_E.begin(), Abilities_A_E.end(), Abilities_A_E[x]));
                                    Save_Setting();
                                }
                            }
                        }
                    }
                    if (!Abilities_P_E.empty()) {
                        for (int x = 0; x < Abilities_P_E.size();x++) {
                            AbilityB.setPosition(Vector2f(view.getCenter().x - 200, view.getCenter().y - 440 + (x * 100) + minusG_CList));
                            if (AbilityB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                                if (Abilities_P_E[x][0] == 'P') {
                                    Abilities_NE.push_back(Abilities_P_E[x]);
                                    Abilities_P_E.erase(find(Abilities_P_E.begin(), Abilities_P_E.end(), Abilities_P_E[x]));
                                    Save_Setting();
                                }
                            }
                        }
                    }
                }
            }
            else if (Setting == true) {
                if (WasFullscreen == Fullscreen) {
                    Write(15, view.getCenter().x - 380, view.getCenter().y - 400, "Полноэкранный режим");
                }
                else{ Write(12, view.getCenter().x - 380, view.getCenter().y - 400, "Полноэкранный режим (перезапустите игру для изменения)"); }
                if (ShadowMove == false) {
                    ShadowMoveB.setTexture(&ForSettingBT1);
                }
                else{ ShadowMoveB.setTexture(&ForSettingBT2); }
                if (ShadowOst == false) {
                    ShadowOstB.setTexture(&ForSettingBT1);
                }
                else { ShadowOstB.setTexture(&ForSettingBT2); }
                if (Fullscreen == false) {
                    FullscreenB.setTexture(&ForSettingBT1);
                }
                else{ FullscreenB.setTexture(&ForSettingBT2); }
                window->draw(BackB);
                window->draw(FullscreenB);
                window->draw(ShadowMoveB);
                Write(15, view.getCenter().x - 380, view.getCenter().y - 370, "Остаточный след за персонажем");
                window->draw(ShadowOstB);
                Write(15, view.getCenter().x - 380, view.getCenter().y - 340, "Остаточное изображение персонажа");
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    if (BackB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        Setting = false;
                    }
                    if (FullscreenB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        Fullscreen = !Fullscreen;
                        Save_Setting();
                    }
                    if (ShadowMoveB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        ShadowMove = !ShadowMove;
                        Save_Setting();
                    }
                    if (ShadowOstB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        ShadowOst = !ShadowOst;
                        Save_Setting();
                    }
                }
            }
            window->display();
        }
        if (Play == true) {
            if (find(Abilities_P_E.begin(), Abilities_P_E.end(), "PLittle") == Abilities_P_E.end()) {
                Player.setSize(Vector2f(20, 20 * 16 / 9));
                moveCooldown = 0.125f;
            }
            else { Player.setSize(Vector2f(10, 10 * 16 / 9));
                   moveCooldown = 0.0625f;
            }
            BlackBlockForPause.setPosition(Vector2f(view.getCenter().x - SizeWin / 2, view.getCenter().y - HeightWin / 2));
            window->draw(BlackBlockForPause);
            window->display();
            running = true;
            if (socket.connect(SERVER_IP, PORT) != Socket::Done) {
                cout << "Ошибка подключения к серверу!" << endl;
                CannotConect = true;
                running = false;
                menu = true;
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }           
            else {
             // Проверяем ответ сервера
                    Packet NO_;
                    if (socket.receive(NO_) == Socket::Done) {
                        string NOorYes;
                        NO_ >> NOorYes;
                        if (NOorYes == "No") {
                            cout << "Сервер переполнен! Попробуйте позже." << endl;
                            socket.disconnect();
                            running = false;
                            menu = true;
                        }
                        else {
                            cout << "Подключено к серверу" << endl;
                            receiverThread = thread(ReceiveMessages, ref(socket));
                        }
                    }
            }
            SendPacket.clear();
            string Hello = "\0";
            Hello = "VD," + to_wstring(Abilities_A_E.size()) + ",";
                for (int x = 0; x < Abilities_A_E.size();x++) {
                    Hello = Hello + Abilities_A_E[x] + ",";
                }
                for (int x = 0; x < Abilities_P_E.size();x++) {
                    Hello = Hello + Abilities_P_E[x] + ",";
                }
                SendPacket << Hello;
                if (socket.send(SendPacket) != Socket::Done && running) {
                    cout << "Ошибка отправки сообщения!" << endl;
                }
            while (running) {
                while (window->pollEvent(event))
                {
                    if (event.type == Event::Closed) {
                        running = false;

                        // Закрываем сокет для разблокировки потока
                        socket.disconnect();

                        // Ожидаем завершение потока
                        if (receiverThread.joinable()) {
                            receiverThread.join();
                        }

                        // Очистка состояния
                        Pause = false;
                        menu = true;
                        Play = false;
                        window->close();
                    }
                }
                MousePos = window->mapPixelToCoords(Mouse::getPosition(*window));

                window->clear();
                Player.setPosition(Vector2f((YourX), (YourY)));

                // хрень для отрисовки врагов
                vector<int> localEnemies;
                {
                    lock_guard<mutex> lock(vectorMutex);
                    localEnemies = XandYother;  // Копируем под защитой мьютекса
                }
                // Отрисовываем из локальной копии
                for (int i = 0; i < localEnemies.size(); i += 2) {
                    if (i + 1 < localEnemies.size()) {
                        if (localEnemies[i] != YourX || localEnemies[i + 1] != YourY) {
                            PlayerEnemy.setPosition(
                                (localEnemies[i]),
                                (localEnemies[i + 1])
                            );
                            window->draw(PlayerEnemy);
                        }
                    }
                }
                //конец хрени для отрисовки врагов

                window->draw(Wall1); window->draw(Wall2); window->draw(Wall3); window->draw(Wall4);

                float Shadow_elapsed = ShadowTimer.getElapsedTime().asSeconds();
                bool Shadow_watch = (Shadow_elapsed >= CanWatchShadow);
                
                if (ShadowMove == true || ShadowOst == true) {
                    if (ShadowMove == true) {
                        if ((Shadow_x == 0 && Shadow_y != 0) || (Shadow_x != 0 && Shadow_y == 0)) {
                            switch (Shadow_x) {
                            case 0: break;
                            case 1:
                                PlayerShadow.setPosition(Vector2f((YourX - 5), (YourY)));
                                PlayerShadow.setSize(Vector2f((5), (20 * 16 / 9)));
                                ShadowTimer.restart();
                                Shadow_x = 0;
                                break;
                            case 2:
                                PlayerShadow.setPosition(Vector2f((YourX + 20), (YourY)));
                                PlayerShadow.setSize(Vector2f((5), (20 * 16 / 9)));
                                ShadowTimer.restart();
                                Shadow_x = 0;
                                break;
                            }
                            switch (Shadow_y) {
                            case 0: break;
                            case 1:
                                PlayerShadow.setPosition(Vector2f((YourX), (YourY + 20 * 16 / 9)));
                                PlayerShadow.setSize(Vector2f((20), (5 * 16 / 9)));
                                ShadowTimer.restart();
                                Shadow_y = 0;
                                break;
                            case 2:
                                PlayerShadow.setPosition(Vector2f((YourX), (YourY - 5 * 16 / 9)));
                                PlayerShadow.setSize(Vector2f((20), (5 * 16 / 9)));
                                ShadowTimer.restart();
                                Shadow_y = 0;
                                break;
                            }
                        }
                        else if (Shadow_x != 0 && Shadow_y != 0) {
                            switch (Shadow_x) {
                            case 1:
                                switch (Shadow_y) {
                                case 1:PlayerShadow.setPosition(Vector2f((YourX - 5), (YourY + 5 * 16 / 9)));
                                    PlayerShadow.setSize(Vector2f((20), (20 * 16 / 9)));
                                    break;
                                case 2:PlayerShadow.setPosition(Vector2f((YourX - 5), (YourY - 5 * 16 / 9)));
                                    PlayerShadow.setSize(Vector2f((20), (20 * 16 / 9)));
                                    break;
                                }
                                break;
                            case 2:
                                switch (Shadow_y) {
                                case 1:PlayerShadow.setPosition(Vector2f((YourX + 5), (YourY + 5 * 16 / 9)));
                                    PlayerShadow.setSize(Vector2f((20), (20 * 16 / 9)));
                                    break;
                                case 2:PlayerShadow.setPosition(Vector2f((YourX + 5), (YourY - 5 * 16 / 9)));
                                    PlayerShadow.setSize(Vector2f((20), (20 * 16 / 9)));
                                    break;
                                }
                                break;
                            }
                            ShadowTimer.restart();
                            Shadow_y = 0;
                            Shadow_x = 0;
                        }
                    }
                    if (YourX != Shadow_X_c || YourY != Shadow_Y_c) {
                        ShadowTimer.restart();
                        PlayerShadowOst.setPosition(Vector2f((Shadow_X_c), (Shadow_Y_c)));
                        Shadow_X_c = YourX;Shadow_Y_c = YourY;
                    }
                    if (Shadow_elapsed <= CanWatchShadow) {
                        if (ShadowMove == true) {
                            window->draw(PlayerShadow);
                        }
                        if (ShadowOst == true) {
                            window->draw(PlayerShadowOst);
                        }
                    }
                }
                window->draw(Player);            
                
                float elapsed = moveTimer.getElapsedTime().asSeconds();
                bool canMove = (elapsed >= moveCooldown);
                if (Pause == false) {
                    if (canMove) {
                        if (GetAsyncKeyState(UP_key) & 0x8000) {
                            YourMessage = YourMessage + "W,";
                            canMove = false;
                        }
                        else if (GetAsyncKeyState(DOWN_key) & 0x8000) {
                            YourMessage = YourMessage + "S,";
                            canMove = false;
                        }
                        if (GetAsyncKeyState(LEFT_key) & 0x8000) {
                            YourMessage = YourMessage + "A,";
                            canMove = false;
                        }
                        else if (GetAsyncKeyState(RIGHT_key) & 0x8000) {
                            YourMessage = YourMessage + "D,";
                            canMove = false;
                        }
                        if (!canMove) {
                            moveTimer.restart();
                        }
                    }
                }
                else if (Pause == true) { BlackBlockForPause.setPosition(Vector2f(view.getCenter().x - SizeWin / 2, view.getCenter().y - HeightWin / 2)); window->draw(BlackBlockForPause);
                PlayB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y - 440));
                ExitB.setPosition(Vector2f(view.getCenter().x - 300, view.getCenter().y + 240));
                window->draw(PlayB);
                window->draw(ExitB);
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    if (PlayB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        Pause = false;
                    }
                    if (ExitB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        running = false;

                        // Закрываем сокет для разблокировки потока
                        socket.disconnect();

                        // Ожидаем завершение потока
                        if (receiverThread.joinable()) {
                            receiverThread.join();
                        }

                        // Очистка состояния
                        Pause = false;
                        menu = true;

                        Cam((SizeWin) / 2, (HeightWin) / 2);
                        window->setView(view);
                    }                
                }
                }

                SendPacket.clear();
                if (YourMessage != "\0") {
                    SendPacket << YourMessage;
                    if (socket.send(SendPacket) != Socket::Done && running) {
                        cout << "Ошибка отправки сообщения!" << endl;
                    }
                    YourMessage = "\0";
                }
                
                Cam((YourX + Player.getSize().x / 2), (YourY + Player.getSize().y  / 2));
                window->setView(view);

                window->display();
            }
            
            if (receiverThread.joinable()) {
                receiverThread.join();
                cout << "Поток приемника завершен" << endl;
            }
            Play = false;
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }        
        Sleep(5);
    }
}