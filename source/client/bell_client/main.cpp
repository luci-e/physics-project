#include <stdio.h>
#include "bApp.h"

int main(int argc, char** argv)
{
	bApp app;
	int r;

	if (r = app.initGL() < 0) {
		return r;
	}

	if (r = app.initSerial() < 0) {
		std::cout << "Could not connect to default serial!" << std::endl;
	}


	app.start();
	app.cleanup();

	return 0;
}