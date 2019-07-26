#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <sstream>
#include <algorithm>    // std::find
#include <cmath>

#include "SerialPort.h"
#include "utilities.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "bApp.h"

int bApp::initGL()
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Bell Theorem", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, cbReshape);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	bool err = glewInit() != GLEW_OK;

	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return -1;

	}

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	const char* glsl_version = "#version 460";

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return 0;
}

int bApp::initSerial()
{
	arduino = new SerialPort( const_cast<char*>( this->portName.c_str() ) );

	int tries = 8;

	//wait connection
	while (!arduino->isConnected() && tries > 0) {
		Sleep(100);
		arduino = new SerialPort(const_cast<char*>(this->portName.c_str()));
		tries--;
	}

	//Checking if arduino is connected or not
	if (arduino->isConnected()) {
		std::cout << "Connection established at port " << portName << std::endl;
		return 0;
	}

	return -1;

}

void bApp::cbReshape(GLFWwindow* window, int x, int y)
{
	bApp& app = *((bApp*)glfwGetWindowUserPointer(window));

	app.wWidth = x;
	app.wHeight = y;
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glfwSwapBuffers(window);
}

void bApp::start()
{
	ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	while (!glfwWindowShouldClose(window)) {
		// update other events like input handling 
		glfwPollEvents();

		// wipe the drawing surface clear
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Connect...")) {
					showInitializerWindow = true;
				}

				if (ImGui::MenuItem("Quit", "Alt+F4")) {
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (ImGui::Begin("Live Data") )
		{
			ImGui::PlotLines("", data, dataSize, dataOffset, NULL, 0.f, 1025.f, ImVec2(512, 256));
			ImGui::Separator();

			ImGui::Text("Stats");
			
			ImGui::Text("Average Value %f", avg);
			ImGui::Text("Passing %f", passing);
			ImGui::Text("Computed Value %f", computed);

			ImGui::Text("Min Value %f", min);
			ImGui::Text("Max Value %f", max);
			ImGui::Text("Percentage %f", 1.f - ( avg - min ) / ( max -min ) );

			ImGui::Separator();
			ImGui::Text("Controls");

			ImGui::Text("Set bounds");

			if (ImGui::Button("Set Min")) {
				min = avg;
			}
			ImGui::SameLine();
			if (ImGui::Button("Set Max")) {
				max = avg;
			}

			ImGui::Text("Polarizer positions");
			for (int i = 0; i < 4; i++)
			{
				char label[32];

				sprintf_s(label, "Angle %d", i);

				ImGui::PushID(i);
				ImGui::SliderAngle(label, &angles[i]);
				ImGui::PopID();
			}

		}
		ImGui::End();

		if (showInitializerWindow) {
			ImGui::Begin("Connect", &showInitializerWindow);

			static char port[4] = "";
			ImGui::Text("Connect to serial");
			ImGui::InputText("port number", port, IM_ARRAYSIZE(port));

			if (ImGui::Button("Connect")) {
				if (strcmp("", port) != 0) {
					portName = std::string("\\\\.\\COM") + port;
					initSerial();
				}
			}
			
			ImGui::End();
		}

		if (arduino->isConnected()) {
			readSerial();
			computeExpected();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}
}

void bApp::readSerial()
{

	int hasRead = arduino->readSerialPort(&serialDataLine[0], sizeof(serialDataLine));
	if (hasRead == 0) {
		return;
	}

	std::string line(serialDataLine);
	std::stringstream ss(line);
	std::string dataPoint = "";

	float next;

	while (std::getline(ss, dataPoint) ) {
		try {
			if (!ss.good()) continue;
			if (dataPoint.length() < 4) continue;

			float next = (float) stoi(dataPoint);
			data[dataOffset] = next;
			dataOffset = (dataOffset + 1) % dataSize;
		}
		catch (std::exception e) {
			data[dataOffset] = avg;
			dataOffset = (dataOffset + 1) % dataSize;
		}
	}

	avg = (float)average(data, dataSize);

}

void bApp::computeExpected()
{
	passing = 1.f;
	float vec[2] = { cos(angles[0]), sin(angles[0]) };

	for (int i = 1; i < 4; i++) {
		float newVec[2] = { cos(angles[i]), sin(angles[i]) };
		float dotProd = dot(vec, newVec, 2);
		dotProd *= dotProd;
		passing *= dotProd;
		memcpy(vec, newVec, 2 * sizeof(float));
	}

	computed = max - (passing * (max - min));
}

void bApp::cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	arduino->closeSerial();

	glfwDestroyWindow(window);
	glfwTerminate();

}
