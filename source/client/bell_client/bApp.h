#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SerialPort.h"
#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>

#include "imgui.h"

#define DATA_SIZE 256

class bApp
{
public:

	GLFWwindow* window;
	std::string portName = "\\\\.\\COM7";

	char serialDataLine[DATA_SIZE];
	float data[DATA_SIZE];
	int dataSize = DATA_SIZE;
	int dataOffset = 0;

	float avg = 0.f;
	float min = 0.f;
	float max = 1024.f;
	float computed = 0.f;
	float passing = 1.f;

	float angles[4] = { 0, 0, 0, 0 };

	SerialPort* arduino;

	bool showInitializerWindow = false;

	// Window properties
	int wWidth = 640, wHeight = 480;

	bApp() {};

	int initGL();
	int initSerial();

	static void cbReshape(GLFWwindow* window, int x, int y);

	void start();

	void readSerial();
	void computeExpected();

	void cleanup();
};

