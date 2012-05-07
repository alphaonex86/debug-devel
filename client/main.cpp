#include <QtGui/QApplication>
#include "mainwindow.h"

/// \brief Entry point
int main(int argc, char *argv[])
{
	//create the main windows
	QApplication a(argc, argv);
	//create and show the main windows
	MainWindow w;
	w.show();
	//enter in the event loop
	return a.exec();
}
