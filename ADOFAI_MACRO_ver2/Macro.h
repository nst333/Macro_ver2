#pragma once
#include "json.h"
#include <vector>
#include <string>
using namespace std;

struct Twirl {
	int floor;
	bool isInside;
/*  ^^^
	inside  (RED) : true
	outside (BLUE) : false
	*/
};

class Macro
{
private:
	Json::Value root;
	vector<double> angle_data;
	vector<Json::Value> actions;
	vector<Twirl> twirl_data;

public:
	void Open(string adofai_file_location);
	void Waiting();
};
