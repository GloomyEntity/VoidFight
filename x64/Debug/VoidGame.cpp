#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <locale>
#include <Windows.h>
#include "H1.h"
#include "Cam.h"
#pragma warning(disable : 4996)

using namespace std;
using namespace sf;

RenderWindow window(VideoMode(SizeWin, HeightWin), L"Новый проект", Style::Default);
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
RectangleShape PlayB(Vector2f(scaleX(600), scaleY(200))); // начать игру
RectangleShape ExitB(Vector2f(scaleX(600), scaleY(200))); // выйти

RectangleShape Player(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin))); // Игрок
RectangleShape PlayerShadow(Vector2f(scaleX(20), scaleY(20))); // тень игрока

RectangleShape Wall1(Vector2f(scaleX(250 * 2), scaleY(10 * SizeWin / HeightWin)));
RectangleShape Wall2(Vector2f(scaleX(10), scaleY(250 * SizeWin / HeightWin * 2)));
RectangleShape Wall3(Vector2f(scaleX(260 * 2 - 10), scaleY(10 * SizeWin / HeightWin)));
RectangleShape Wall4(Vector2f(scaleX(10), scaleY(250 * SizeWin / HeightWin * 2)));

const string SERVER_IP = "26.148.185.163";
const unsigned short PORT = 62136;

atomic<bool> running(false);

Packet SendPacket; // Для отправки (главный поток)
Packet ReceivePacket;
string YourMessage;
string ServerMessage;

Clock moveTimer;
const float moveCooldown = 0.125f;
Clock ShadowTimer;
const float CanWatchShadow = 0.2f;

RectangleShape BlackBlockForPause(Vector2f(SizeWin, HeightWin));
void CorrectWind() {
    while (window.isOpen()) {
        Sleep(10);
        if (window.getSize().x != SizeWin || window.getSize().y != HeightWin) {
            SizeWin = window.getSize().x;
            HeightWin = window.getSize().y;

            // Масштабируем все элементы
            PlayB.setSize(Vector2f(scaleX(600), scaleY(200)));
            ExitB.setSize(Vector2f(scaleX(600), scaleY(200)));
            Player.setSize(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin)));
            PlayerShadow.setSize(Vector2f(scaleX(50), scaleY(50)));

            Wall1.setSize(Vector2f(scaleX(250 * 2), scaleY(10 * SizeWin / HeightWin)));
            Wall2.setSize(Vector2f(scaleX(10), scaleY(250 * SizeWin / HeightWin * 2)));
            Wall3.setSize(Vector2f(scaleX(260 * 2 - 10), scaleY(10) * SizeWin / HeightWin));
            Wall4.setSize(Vector2f(scaleX(10), scaleY(250 * SizeWin / HeightWin * 2)));

            PlayB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y - scaleY(440)));
            ExitB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y + scaleY(240)));

            Player.setPosition(Vector2f(scaleX(YourX), scaleY(YourY)));

            BlackBlockForPause.setSize(Vector2f(SizeWin, HeightWin));

            Wall1.setPosition(Vector2f(scaleX(-100 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));
            Wall2.setPosition(Vector2f(scaleX(150 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));
            Wall3.setPosition(Vector2f(scaleX(-100 * 2), scaleY(150 * SizeWin / HeightWin * 2)));
            Wall4.setPosition(Vector2f(scaleX(-100 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));
        }
    }
}



void KeysClient() {
    while(true){
        Sleep(10);
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                if (Pause == true) {
                    Pause = false;
                }
                else { Pause = true; }
                keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
            }
    }
}

void ReceiveMessages(TcpSocket& socket) {
    while (running) {
        if (socket.receive(ReceivePacket) == Socket::Done) {
            ReceivePacket >> ServerMessage;
            cout << ServerMessage << endl; 
            if (ServerMessage[0] == 'V') {
                int NumS = 1;
                int NumW = 0;
                string wordM;
                while (NumS < size(ServerMessage)) {
                    if (ServerMessage[NumS] == ',') {
                        switch (NumW) {
                        case 0:
                            if (YourX != stoi(wordM)) {
                                if (YourX < stoi(wordM)) {
                                    Shadow_x = 1;
                                }
                                else { Shadow_x = 2; }
                            }
                            YourX = stoi(wordM);
                            break;
                        case 1:
                            if (YourY != stoi(wordM)) {
                                if (YourY > stoi(wordM)) {
                                    Shadow_y = 1;
                                }
                                else { Shadow_y = 2; }
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
            ReceivePacket.clear();
            ServerMessage = "\0";
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

thread receiverThread;

int main() {
    thread Keys_C(KeysClient);
    Keys_C.detach();

    Wall1.setPosition(Vector2f(scaleX(-100 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));
    Wall2.setPosition(Vector2f(scaleX(150 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));
    Wall3.setPosition(Vector2f(scaleX(-100 * 2), scaleY(150 * SizeWin / HeightWin * 2)));
    Wall4.setPosition(Vector2f(scaleX(-100 * 2), scaleY(-100 * SizeWin / HeightWin * 2)));

    setlocale(LC_ALL, "RU");
    TcpSocket socket;

    Cam(scaleX(SizeWin) / 2, scaleY(HeightWin) / 2);
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

    Player.setFillColor(Color::White);
    PlayerShadow.setFillColor(Color::White);

    BlackBlockForPause.setFillColor(Color(0, 0, 0, 100));

    while (true) {
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                running = false;
                window.close();
            }
        }
        if (menu == true) {  
            window.clear();
            MousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            Cam(scaleX(SizeWin) / 2, scaleY(HeightWin) / 2);
            window.setView(view);
            PlayB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y - scaleY(440)));
            ExitB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y + scaleY(240)));
            window.draw(PlayB);
            window.draw(ExitB);
            
            if (Mouse::isButtonPressed(Mouse::Left)) {
                if (PlayB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    Play = true;
                    menu = false;
                }
                if (ExitB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    window.close();
                    exit(EXIT_SUCCESS);
                }              
            }
            window.display();
        }
        if (Play == true) {
            running = true;
            if (socket.connect(SERVER_IP, PORT) != Socket::Done) {
                cerr << "Ошибка подключения к серверу!" << endl;
                running = false;
                menu = true;
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
                SendPacket << "VD";
                if (socket.send(SendPacket) != Socket::Done && running) {
                    cerr << "Ошибка отправки сообщения!" << endl;
                }
            while (running) {
                window.clear();
                Player.setPosition(Vector2f(scaleX(YourX), scaleY(YourY)));
                MousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                window.draw(Wall1); window.draw(Wall2); window.draw(Wall3); window.draw(Wall4);

                float Shadow_elapsed = ShadowTimer.getElapsedTime().asSeconds();
                bool Shadow_watch = (Shadow_elapsed >= CanWatchShadow);
                
                if ((Shadow_x == 0 && Shadow_y != 0) || (Shadow_x != 0 && Shadow_y == 0)) {
                    switch (Shadow_x) {
                    case 0: break;
                    case 1: 
                        PlayerShadow.setPosition(Vector2f(scaleX(YourX - 10), scaleY(YourY)));
                        PlayerShadow.setSize(Vector2f(scaleX(10), scaleY(20 * SizeWin / HeightWin)));
                        ShadowTimer.restart();
                        Shadow_x = 0;
                        break;
                    case 2: 
                        PlayerShadow.setPosition(Vector2f(scaleX(YourX + 20), scaleY(YourY)));
                        PlayerShadow.setSize(Vector2f(scaleX(10), scaleY(20 * SizeWin / HeightWin)));
                        ShadowTimer.restart();
                        Shadow_x = 0;
                        break;
                    }
                    switch (Shadow_y) {
                    case 0: break;
                    case 1: 
                        PlayerShadow.setPosition(Vector2f(scaleX(YourX), scaleY(YourY + 20 * SizeWin / HeightWin)));
                        PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(10 * SizeWin / HeightWin)));
                        ShadowTimer.restart();
                        Shadow_y = 0;
                        break;
                    case 2: 
                        PlayerShadow.setPosition(Vector2f(scaleX(YourX), scaleY(YourY - 10 * SizeWin / HeightWin)));
                        PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(10 * SizeWin / HeightWin)));
                        ShadowTimer.restart();
                        Shadow_y = 0;
                        break;
                    }
                }
                else if (Shadow_x != 0 && Shadow_y != 0) {
                    switch (Shadow_x) {
                    case 1: 
                        switch (Shadow_y) {
                        case 1:PlayerShadow.setPosition(Vector2f(scaleX(YourX - 5), scaleY(YourY + 10 * SizeWin / HeightWin)));
                            PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin)));
                            break;
                        case 2:PlayerShadow.setPosition(Vector2f(scaleX(YourX - 5), scaleY(YourY - 5 * SizeWin / HeightWin)));
                            PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin)));
                            break;
                        }
                        break;
                    case 2:
                        switch (Shadow_y) {
                        case 1:PlayerShadow.setPosition(Vector2f(scaleX(YourX + 10), scaleY(YourY + 10 * SizeWin / HeightWin)));
                            PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin)));
                            break;
                        case 2:PlayerShadow.setPosition(Vector2f(scaleX(YourX + 10), scaleY(YourY - 5 * SizeWin / HeightWin)));
                            PlayerShadow.setSize(Vector2f(scaleX(20), scaleY(20 * SizeWin / HeightWin)));
                            break;
                        }
                        break;
                    }
                    ShadowTimer.restart();
                    Shadow_y = 0;
                    Shadow_x = 0;
                }
                
                if (Shadow_elapsed <= CanWatchShadow) {
                    window.draw(PlayerShadow);
                }
                window.draw(Player);
                
                while (window.pollEvent(event))
                {
                    if (event.type == Event::Closed) {
                        running = false;
                        window.close();
                    }
                }
                
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
                else if (Pause == true) { BlackBlockForPause.setPosition(Vector2f(view.getCenter().x - SizeWin / 2, view.getCenter().y - HeightWin / 2)); window.draw(BlackBlockForPause);
                PlayB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y - scaleY(440)));
                ExitB.setPosition(Vector2f(view.getCenter().x - scaleX(300), view.getCenter().y + scaleY(240)));
                window.draw(PlayB);
                window.draw(ExitB);
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    if (PlayB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        Pause = false;
                    }
                    if (ExitB.getGlobalBounds().contains(MousePos.x, MousePos.y)) {
                        // Корректное завершение
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
                    }                
                }
                }

                SendPacket.clear();
                if (YourMessage != "\0") {
                    SendPacket << YourMessage;
                    if (socket.send(SendPacket) != Socket::Done && running) {
                        cerr << "Ошибка отправки сообщения!" << endl;
                    }
                    YourMessage = "\0";
                }
                
                Cam(scaleX(YourX + Player.getSize().x / 2), scaleY(YourY + Player.getSize().y  / 2));
                window.setView(view);

                window.display();
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