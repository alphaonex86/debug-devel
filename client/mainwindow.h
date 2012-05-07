/***************************************************************************
                                mainwindows.h
                              -------------------

     Begin        : Mon Nov 29 2008 18:20 alpha_one_x86
     Project      : debug-devel
     Email        : alpha.super-one@laposte.net
     Note         : See doc for copyright and developer
     Target       : Class for do testing of communication

****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#define RS232_USE

#include <QtGui/QMainWindow>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QMessageBox>
#include <QByteArray>
#include <QString>
#include <QFileDialog>
#include <QHostAddress>
#include <QIODevice>
#include <zlib.h>
#include <qtiocompressor.h>
#include <QBuffer>
#include "lz4.h"
#include "lz4hc.h"

#ifdef RS232_USE
#include <qextserialport.h>
#include <qextserialenumerator.h>
	#ifndef RS232MODE_EVENT
	#include <QTimer>
	#endif // RS232MODE_EVENT
#include "rs232-mode.h"
#endif // RS232_USE

namespace Ui {
	class MainWindow;
}

/// \brief Class for do testing of communication
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	//create main windows and configure it
	MainWindow(QWidget *parent = 0);
	//disconnect of all the socket and unallocate the memory
	~MainWindow();
private:
	Ui::MainWindow *ui;
	bool isConnected;		///< Store status of connection
	QString connectedMode;		///< Store onnection mode
	QTcpSocket *tcpSocket;		///< Store tcp socket
	QLocalSocket *localSocket;	///< Store local socket
	#ifdef RS232_USE
	QextSerialPort *port;		///< Store com rs232
		#ifndef RS232MODE_EVENT
		QTimer *pollingTimer;		///< For look is new data incomming in polling mode
		QByteArray RS232data;		///< Store the incoming RS232 data
		#endif // RS232MODE_EVENT
	QList<QextPortInfo> ports;	///< Store port found
	#endif // RS232_USE
	QByteArray IncomingData;	///< Store the incoming data
	QByteArray OutgoingData;	///< Store the outgoing data
	QString LastEditionMode;	///< Store the last edition mode used for Tx
	QByteArray decompress(QByteArray compressed_data);
	QByteArray compress(QByteArray raw_data);
	QtIOCompressor* decompressor;
	QBuffer* buffer_decompression;
	QByteArray buffer_decompression_out;
	QtIOCompressor* compressor;
	QBuffer* buffer_compression;
	QByteArray buffer_compression_out;
private slots:
	//look the mode update information and widget displayed
	void UpdateMode();
	//try disconnect the current connexion
	bool Disconnect();
	//try connect, disconnect if needed
	bool Connect();
	//call this function when connection button is clicked
	void ActionClickedConnectButton();
	//update the connexion button and information displayed on it
	void updateConnectButton();
	//try send new data given
	bool sendNewData();
	//call this function when new data is incomming
	bool newDataIncomming();
	//call this function when error on tcpSocket append
	bool errorUI(QAbstractSocket::SocketError);
	//call this function when error on localSocket append
	bool errorUI(QLocalSocket::LocalSocketError);
	//serveur have close the connection
	bool deconnectClientUI();
	//call when is connected on serveur local or tcp
	bool nowConnected();
	//mode Rx have changed
	bool ModeRxHaveChanged(QString);
	//mode Tx have changed
	bool ModeTxHaveChanged(QString);
	//clear the Rx field
	void ClearTheRxField();
	//clear the Tx field
	void ClearTheTxField();
	//update the Tx field
	void UpdateTheTxField();
	#ifdef RS232_USE
	#ifndef RS232MODE_EVENT
	void lookNewDataRS232();
	#endif // RS232MODE_EVENT
	//update port list
	void updatePortList();
	#endif // RS232_USE
	//load the Tx file
	void loadTheFileTx();
	//save the Rx file
	void saveTheFileRx();
};

#endif // MAINWINDOW_H


