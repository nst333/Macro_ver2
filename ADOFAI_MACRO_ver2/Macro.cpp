#include "Macro.h"
#include <map>
#include <iostream>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <Psapi.h>
#include <tchar.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
using namespace std;
using namespace cv;
using namespace chrono;

#define ONE_HAND_BPM (60000.0 / 1200.0)
#define ONLY_ONE_HAND false
const int key_table[8] = { 'J', 'O', 'P', VK_OEM_7, 'F', 'E', 'W', 'A' };

#define GetRValue( rgb )	( ( BYTE )( rgb ) )
#define GetGValue( rgb )	( ( BYTE )( ( ( WORD )( rgb ) ) >> 8 ) )
#define GetBValue( rgb )	( ( BYTE )( ( rgb ) >> 16 ) )

void Macro::Open(string adofai_file_location) {
	Json::CharReaderBuilder reader;
	ifstream is(adofai_file_location, ifstream::binary);
	string errorMessage;
	auto bret = Json::parseFromStream(reader, is, &this->root, &errorMessage);

	if (bret == false) {
		cout << "Error to parse JSON file !!!" << " " << errorMessage << endl;
		return;
	}

	if (this->root["angleData"].size() == 0) {
		string path = this->root["pathData"].asString();

		map<char, int> direction;
		direction.insert(pair<char, int>('R', 1));
		direction.insert(pair<char, int>('p', 2));
		direction.insert(pair<char, int>('J', 3));
		direction.insert(pair<char, int>('E', 4));
		direction.insert(pair<char, int>('T', 5));
		direction.insert(pair<char, int>('o', 6));
		direction.insert(pair<char, int>('U', 7));
		direction.insert(pair<char, int>('q', 8));
		direction.insert(pair<char, int>('G', 9));
		direction.insert(pair<char, int>('Q', 10));
		direction.insert(pair<char, int>('H', 11));
		direction.insert(pair<char, int>('W', 12));
		direction.insert(pair<char, int>('L', 13));
		direction.insert(pair<char, int>('x', 14));
		direction.insert(pair<char, int>('N', 15));
		direction.insert(pair<char, int>('Z', 16));
		direction.insert(pair<char, int>('F', 17));
		direction.insert(pair<char, int>('V', 18));
		direction.insert(pair<char, int>('D', 19));
		direction.insert(pair<char, int>('Y', 20));
		direction.insert(pair<char, int>('B', 21));
		direction.insert(pair<char, int>('C', 22));
		direction.insert(pair<char, int>('M', 23));
		direction.insert(pair<char, int>('A', 24));
		direction.insert(pair<char, int>('!', -1));

		for (int i = 0; i < path.size(); i++) {
			this->root["angleData"][i] = (direction[path[i]] - 1) * 15;
			if (direction[path[i]] == -1) {
				this->root["angleData"][i] = 999;
			}
		}
	}

	this->root.removeMember("pathData");

	Json::Value angleData = this->root["angleData"];

	this->angle_data.clear();
	for (int i = 0; i < angleData.size(); i++) {
		this->angle_data.push_back(angleData[i].asDouble());
	}

	// Save actions
	// Save Action Type : vector<Json::Value>
	vector<Json::Value> actions;
	for (int i = 0; i < this->root["actions"].size(); i++) {
		actions.push_back(this->root["actions"][i]);
	}

	/*for (vector<Json::Value>::iterator iter = actions.begin(); iter != actions.end(); ++iter) {
		cout << *iter << endl;
	}*/

	cout << actions.size() << endl;
	cout << angleData.size() << endl;
	int twirlEventCount = 0; // "Twirl" 이벤트 개수 저장 변수

	for (int i = 0; i < actions.size(); i++) {

		if (actions[i]["eventType"] == "Twirl") {
			struct Twirl twirl_s;
			twirl_s.floor = actions[i]["floor"].asInt();
			twirl_s.isInside = false; // 요소 초기화

			bool isInside = true;

			if (this->twirl_data.size() >= 1) {
				isInside = !this->twirl_data[i].isInside;
			}
			else {
				isInside = !isInside;
			}

			twirl_s.isInside = isInside;
			this->twirl_data.push_back(twirl_s);

			twirlEventCount++;
		}
	}

	// Output
	for (int i = 0; i < this->twirl_data.size(); i++) {
		if (this->twirl_data[i].isInside) {
			cout << this->twirl_data[i].floor << " | " << this->twirl_data[i].isInside << endl;
		}
		else {
			cout << this->twirl_data[i].floor << " | " << this->twirl_data[i].isInside << endl;
		}
	}
}

void Macro::Waiting() {

	// Find the process window
	HWND hwnd = FindWindow(NULL, L"A Dance of Fire and Ice");
	if (hwnd == 0)
	{
		std::cout << "Failed to find the window handle of '" << "A Dance of Fire and Ice" << "'" << std::endl;
		return;
	}

	// Get the window dimensions
	RECT windowRect;
	GetClientRect(hwnd, &windowRect);
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Create a blank image to hold the pixel values
	cv::Mat image(windowHeight, windowWidth, CV_8UC3);

	while (true)
	{
		// Capture the process window
		HDC hdcWindow = GetDC(hwnd);
		HDC hdcCompatible = CreateCompatibleDC(hdcWindow);
		HBITMAP hbitmap = CreateCompatibleBitmap(hdcWindow, windowWidth, windowHeight);
		SelectObject(hdcCompatible, hbitmap);
		BitBlt(hdcCompatible, 0, 0, windowWidth, windowHeight, hdcWindow, 0, 0, SRCCOPY);
		GetDIBits(hdcCompatible, hbitmap, 0, windowHeight, image.data, (BITMAPINFO*)&image, DIB_RGB_COLORS);
		DeleteDC(hdcCompatible);
		ReleaseDC(hwnd, hdcWindow);
		DeleteObject(hbitmap);

		// Display the image with OpenCV
		cv::imshow("Process Window", image);

		// Access and process the pixel values
		for (int y = 0; y < windowHeight; ++y)
		{
			for (int x = 0; x < windowWidth; ++x)
			{
				// Get the BGR pixel values
				cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
				int blue = pixel[0];
				int green = pixel[1];
				int red = pixel[2];

				// Do something with the pixel values
				// For example, print them
				std::cout << "Pixel at (" << x << ", " << y << "): (B=" << blue << ", G=" << green << ", R=" << red << ")" << std::endl;
			}
		}

		// Check for ESC key press to exit the loop
		if (cv::waitKey(1) == 27)
			break;
	}

	return;
}
