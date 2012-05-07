#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QList>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QByteArray>
#include <QString>
#include <QTcpServer>
#include <QLocalServer>
#include <QListWidgetItem>
#include <zlib.h>
#include <qtiocompressor.h>
#include <QBuffer>
#include "lz4.h"
#include "lz4hc.h"

int QLZ4_uncompress_unknownOutputSize(QByteArray *source,QByteArray *destination,int maxOutputSize);

typedef struct {
QTcpSocket		*tcpSocket;
QByteArray		IncomingData;
QByteArray		OutgoingData;
QString			LastEditionMode;
QListWidgetItem		*ItemInList;
bool			isConnected;
QtIOCompressor* decompressor;
QBuffer* buffer_decompression;
QByteArray buffer_decompression_out;
QtIOCompressor* compressor;
QBuffer* buffer_compression;
QByteArray buffer_compression_out;
} tcpClientInfo;

typedef struct {
QLocalSocket		*localSocket;
QByteArray		IncomingData;
QByteArray		OutgoingData;
QString			LastEditionMode;
QListWidgetItem		*ItemInList;
bool			isConnected;
QtIOCompressor* decompressor;
QBuffer* buffer_decompression;
QByteArray buffer_decompression_out;
QtIOCompressor* compressor;
QBuffer* buffer_compression;
QByteArray buffer_compression_out;
} localClientInfo;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
	bool			isConnected;		///< Store status of connection
	QString			connectedMode;		///< Store onnection mode
	QList<tcpClientInfo*>	tcpSocket;		///< Store tcp socket
	QList<localClientInfo*>	localSocket;		///< Store local socket
	QTcpServer		TcpServer;
	QLocalServer		LocalServer;
	int			idConnection;

private slots:
	void on_pushButtonSend_clicked();
	void on_comboBoxTypeRx_currentIndexChanged(QString );
	void on_listWidgetClient_itemSelectionChanged();
	void on_toolButton_clicked();
	void on_comboBoxTypeTx_currentIndexChanged(QString );
	void on_textEditToClient_textChanged();
	void on_pushButtonConnect_clicked();
	void on_ClearDataFromClient_clicked();
	void on_ClearDataToClient_clicked();
	void on_SaveFile_clicked();
	void on_LoadFile_clicked();
	void updateMode();
	void updateConnectButton();
	bool ModeRxHaveChanged(QString newMode);
	// void UpdateTheTxField();
	bool Disconnect();
	bool Connect();
	void newClient();
	void clientLeave();
	void newErrorLocal(QLocalSocket::LocalSocketError error);
	void newErrorTcp(QAbstractSocket::SocketError error);
	int indexOfSelection();
	void newData();
	void updateCurrentData();
	bool ModeTxHaveChanged(QString newMode);
	bool sendNewData();
	QByteArray decompress(QByteArray compressed_data,QBuffer *buffer_decompression,QByteArray *buffer_decompression_out,QtIOCompressor* decompressor);
	QByteArray compress(QByteArray raw_data,QBuffer *buffer_compression,QByteArray *buffer_compression_out,QtIOCompressor* compressor);
};

#endif // MAINWINDOW_H
