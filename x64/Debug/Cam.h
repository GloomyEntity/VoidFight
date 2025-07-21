#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

View view;

View Cam(int x, int y) {
	view.setCenter(x, y);
	return view;
}