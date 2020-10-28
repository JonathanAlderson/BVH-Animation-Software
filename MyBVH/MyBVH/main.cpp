///////////////////////////////////////////////////
//
//	Hamish Carr
//	January, 2018
//
//	------------------------
//	main.cpp
//	------------------------
//
///////////////////////////////////////////////////

#include <QApplication>
#include "RenderWidget.h"
#include "MasterWidget.h"
#include <stdio.h>

#define SCREENX 900
#define SCREENY 600

int main(int argc, char **argv)
	{ // main()
	// initialize QT
	QApplication app(argc, argv);

	// Set the filename to empty string if not provided
	char * filename = argv[1];
	char empty = '\0';
	if(argc < 2){ filename = &empty; }

	//	create a window, passing the file name
	MasterWidget aWindow(filename, NULL);

	// 	set the initial size
	aWindow.resize(SCREENX, SCREENY);

	aWindow.show();

	// set QT running
	return app.exec();

	// paranoid return value
	exit(0);
	} // main()
