#pragma once
#include <opencv2/opencv.hpp>

enum class State {
	DISPLAY_CONNECT_MENU,
	START_THREADS,
	DISPLAY_IMAGE,
	QUIT,
};

enum class ConnectionState {
	NOT_YET,
	FAIL,
	SUCCESS,
};

enum class DiscoverState {
	NOT_YET,
	SUCCESS,
};

void initUI();
void freeUI();
void displayConnectMenu();
void renderControlPanel();
void renderImage(cv::Mat image);
void receiveAndDisplayImage();