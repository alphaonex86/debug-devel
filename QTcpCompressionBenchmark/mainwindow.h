#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QTime>

#include "CompressionTcpSocket/CompressionTcpSocketInterface.h"
#include "CompressionTcpSocket/NoCompressionTcpSocket.h"
#include "CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.h"
#include "CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.h"
#include "CompressionTcpSocket/zlib/GzipCompressionTcpSocket.h"
#include "CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	struct data
	{
		QByteArray data;
		QString name;
		int count;
	};
	QList<data> data_list;
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
private slots:
	void on_add_clicked();
	void on_remove_clicked();
	void on_load_clicked();
	void on_save_clicked();
	void on_benchmark_clicked();
	void on_edit_clicked();
private:
	CompressionTcpSocketInterface *compressor;
	CompressionTcpSocketInterface *decompressor;
	Ui::MainWindow *ui;
	void updateShowedItems();
};

#endif // MAINWINDOW_H
