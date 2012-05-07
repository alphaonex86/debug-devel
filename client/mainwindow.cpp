/***************************************************************************
                                mainwindows.cpp
                              -------------------

     Begin        : Mon Nov 29 2008 18:20 alpha_one_x86
     Project      : debug-devel
     Email        : alpha.super-one@laposte.net
     Note         : See doc for copyright and developer
     Target       : Class for do testing of communication

****************************************************************************/

#include "mainwindow.h"
#include "ui_MainWindow.h"

#include "lz4.h"

/// \brief look the mode update information and widget displayed
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->comboBoxTypeTx->setCurrentIndex(1);
	ui->comboBoxTypeRx->setCurrentIndex(1);
	UpdateMode();
	connect(ui->comboBoxMode,SIGNAL(currentIndexChanged(int)),this,SLOT(UpdateMode()));
	connect(ui->pushButtonConnect,SIGNAL(clicked()),this,SLOT(ActionClickedConnectButton()));
	connect(ui->pushButtonSend,SIGNAL(clicked()),this,SLOT(sendNewData()));
	connectedMode="";
	isConnected=false;
	buffer_compression=new QBuffer(&buffer_compression_out);
	compressor=NULL;//allocated later for the buffer
	buffer_decompression=new QBuffer(&buffer_decompression_out);
	decompressor=NULL;//allocated later for the buffer
	
	tcpSocket = new QTcpSocket();
	connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(newDataIncomming()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorUI(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(deconnectClientUI()));
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(nowConnected()));
	
	localSocket = new QLocalSocket();
	connect(localSocket,SIGNAL(readyRead()),this,SLOT(newDataIncomming()));
	connect(localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(errorUI(QLocalSocket::LocalSocketError)));
	connect(localSocket, SIGNAL(disconnected()), this, SLOT(deconnectClientUI()));
	connect(localSocket, SIGNAL(connected()), this, SLOT(nowConnected()));
	
	#ifdef RS232_USE
	#ifdef RS232MODE_EVENT
	//modify the port settings on your own
	#ifdef _TTY_POSIX_
		port = new QextSerialPort(QextSerialBase::EventDriven);
	#else
		port = new QextSerialPort(QextSerialBase::EventDriven);
	#endif /*_TTY_POSIX*/
	#else // RS232MODE_EVENT
	//modify the port settings on your own
	#ifdef _TTY_POSIX_
		port = new QextSerialPort(QextSerialBase::Polling);
	#else
		port = new QextSerialPort(QextSerialBase::Polling);
	#endif /*_TTY_POSIX*/
	#endif // RS232MODE_EVENT

	connect(port,SIGNAL(readyRead()),this,SLOT(newDataIncomming()));
	
	port->setDataBits(DATA_8);
	#ifndef RS232MODE_EVENT
	//set timeouts to 50 ms
	port->setTimeout(50);
	pollingTimer=new QTimer();
	pollingTimer->setInterval(50);
	connect(pollingTimer,SIGNAL(timeout()),this,SLOT(lookNewDataRS232()));
	#endif // RS232MODE_EVENT
	
	//disable speed unvalable
	#ifdef _TTY_POSIX_
		comboBoxSpeed->setItemIcon(13,QIcon(":/images/warning.png"));
		comboBoxSpeed->setItemIcon(16,QIcon(":/images/warning.png"));
		comboBoxSpeed->setItemIcon(20,QIcon(":/images/warning.png"));
		comboBoxSpeed->setItemIcon(21,QIcon(":/images/warning.png"));
		
		comboBoxParity->setItemIcon(3,QIcon(":/images/not-valide.png"));
		
		comboBoxStopBit->setItemIcon(1,QIcon(":/images/not-valide.png"));
	#else
		comboBoxSpeed->setItemIcon(0,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(1,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(3,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(4,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(5,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(9,QIcon(":/images/not-valide.png"));
		comboBoxSpeed->setItemIcon(18,QIcon(":/images/not-valide.png"));
	#endif /*_TTY_POSIX*/
	updatePortList();
	#else
	ui->comboBoxMode->removeItem(0);
	#endif // RS232_USE

	connect(ui->comboBoxTypeRx,SIGNAL(currentIndexChanged(QString)),this,SLOT(ModeRxHaveChanged(QString)));
	connect(ui->comboBoxTypeTx,SIGNAL(currentIndexChanged(QString)),this,SLOT(ModeTxHaveChanged(QString)));
	connect(ui->toolButtonClearRx,SIGNAL(clicked()),this,SLOT(ClearTheRxField()));
	connect(ui->toolButtonClearTx,SIGNAL(clicked()),this,SLOT(ClearTheTxField()));
	connect(ui->plainTextEditTx,SIGNAL(textChanged()),this,SLOT(UpdateTheTxField()));
	connect(ui->loadFileTx,SIGNAL(clicked()),this,SLOT(loadTheFileTx()));
	connect(ui->saveFileRx,SIGNAL(clicked()),this,SLOT(saveTheFileRx()));
}

/// \brief try disconnect the current connexion
MainWindow::~MainWindow()
{
	Disconnect();
	localSocket->deleteLater();
	tcpSocket->deleteLater();
	#ifdef RS232_USE
	port->deleteLater();
	#ifndef RS232MODE_EVENT
	pollingTimer->deleteLater();
	#endif // RS232MODE_EVENT
	#endif // RS232_USE
}

/** \brief call when is connected on serveur local or tcp
\return Return true if is connected */
bool MainWindow::nowConnected()
{
	qDebug() << "nowConnected():" << connectedMode;
	if(connectedMode=="Tcp Socket") // if connection is on tcp socket mode
	{
		qDebug() << "QAbstractSocket::ConnectedState:" << QAbstractSocket::ConnectedState;
		//check if is connected
		isConnected=(tcpSocket->state()==QAbstractSocket::ConnectedState);
		if(ui->compressedStream->isChecked())
		{
		}
		//check if valid
		if(!tcpSocket->isValid())
		{
			ui->statusBarApp->showMessage(tr("tcpSocket is not valid!"));
			isConnected=false;
			return false;
		}
		//check if error is found
		if(tcpSocket->errorString()!="Unknown error" && tcpSocket->errorString()!="" && tcpSocket->state()!=QAbstractSocket::ConnectedState)
		{
			ui->statusBarApp->showMessage(tr("tcpSocket->errorString(): ")+tcpSocket->errorString());
			return false;
		}
		//update the status of the connexion
		if(isConnected)
		{
			connectedMode="Tcp Socket";
			ui->statusBarApp->showMessage(tr("Connected on ")+tcpSocket->peerAddress().toString()+":"+QString::number(tcpSocket->peerPort()));
			updateConnectButton();
			return true;
		}
		else
		{
			ui->statusBarApp->showMessage(tr("Fail connection on ")+tcpSocket->peerAddress().toString()+":"+QString::number(tcpSocket->peerPort())+" "+tcpSocket->errorString());
			connectedMode="";
			return false;
		}
	}
	else if(connectedMode=="Local Socket") // if connection is on local socket mode
	{
		qDebug() << "QLocalSocket::ConnectedState:" << QLocalSocket::ConnectedState;
		//check if is connected
		isConnected=(localSocket->state()==QLocalSocket::ConnectedState);
		if(ui->compressedStream->isChecked())
		{
		}
		//check if valid
		if(!localSocket->isValid())
		{
			ui->statusBarApp->showMessage(tr("localSocket is not valid!"));
			return false;
		}
		//check if error is found
		if(localSocket->errorString()!="Unknown error" && localSocket->errorString()!="" && localSocket->state()!=QLocalSocket::ConnectedState)
		{
			ui->statusBarApp->showMessage(tr("localSocket->errorString(): ")+localSocket->errorString());
			return false;
		}
		//update the status of the connexion
		if(isConnected)
		{
			connectedMode="Local Socket";
			ui->statusBarApp->showMessage(tr("Connected on ")+localSocket->serverName());
			updateConnectButton();
			return true;
		}
		else
		{
			ui->statusBarApp->showMessage(tr("Fail connection on ")+localSocket->serverName()+" "+localSocket->errorString());
			connectedMode="";
			return false;
		}
	}
	return false;
}

/** \brief try disconnect the current connexion
\return Return true if disconnect */
bool MainWindow::Disconnect()
{
	#ifdef RS232_USE
	updatePortList();
	#endif // RS232_USE
	qDebug() << "Disconnect(" << connectedMode << "): check state:";
	qDebug() << "isConnected:" << isConnected << "tcpSocket->state():" << tcpSocket->state() << "localSocket->state():" << localSocket->state();
	//disconnect only if connected
	if(!isConnected && tcpSocket->state()==QAbstractSocket::UnconnectedState && localSocket->state()==QLocalSocket::UnconnectedState)
	{
		ui->statusBarApp->showMessage(tr("Already disconnected"));
		return false;
	}
	qDebug() << "Try disconnection...";

	if(ui->compressedStream->isChecked())
	{
	}

	if(connectedMode=="Tcp Socket") // if connection is on tcp socket mode
	{
		//try disconnection
		tcpSocket->disconnectFromHost();
		if(tcpSocket->state()==QAbstractSocket::ClosingState)
			tcpSocket->waitForDisconnected();
		tcpSocket->abort();

		disconnect(tcpSocket);
		delete tcpSocket;
		tcpSocket = new QTcpSocket();
		connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(newDataIncomming()));
		connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorUI(QAbstractSocket::SocketError)));
		connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(deconnectClientUI()));
		connect(tcpSocket, SIGNAL(connected()), this, SLOT(nowConnected()));
		qDebug() << "tcpSocket->state():" << tcpSocket->state();
		//check if is disconnected
		if(tcpSocket->state()==QAbstractSocket::UnconnectedState)
		{
			isConnected=(tcpSocket->state()!=QAbstractSocket::UnconnectedState);
			if(!isConnected)
			{
				connectedMode="";
				ui->statusBarApp->showMessage(tr("Disconnected"));
			}
			updateConnectButton();
			return true;
		}
		ui->statusBarApp->showMessage(tr("Not disconnected: ")+tcpSocket->errorString());
		return false;
	}
	else if(connectedMode=="Local Socket") // if connection is on local socket mode
	{
		//try disconnection
		localSocket->disconnectFromServer();
		if(localSocket->state()==QLocalSocket::ConnectingState)
			localSocket->waitForDisconnected();
		localSocket->abort();
		//check if is disconnected
		if(localSocket->state()==QLocalSocket::UnconnectedState)
		{
			isConnected=(localSocket->state()!=QLocalSocket::UnconnectedState);
			if(!isConnected)
			{
				connectedMode="";
				ui->statusBarApp->showMessage(tr("Disconnected"));
			}
			updateConnectButton();
			return true;
		}
		ui->statusBarApp->showMessage(tr("Not disconnected: ")+localSocket->errorString());
		return false;
	}
	#ifdef RS232_USE
	else if(connectedMode=="RS232") // if connection is on com rs232 mode
	{
		//try close the port
		port->close();
		//check if is close
		isConnected=port->isOpen();
		if(!isConnected)
		{
			#ifndef RS232MODE_EVENT
			pollingTimer->stop();	//stop polling
			#endif // RS232MODE_EVENT
			connectedMode="";
			statusBarApp->showMessage(tr("Port closed"));
		}
		return true;
	}
	#endif // RS232_USE
	return false;
}

//try send new data given
bool MainWindow::sendNewData()
{
	//check if it connected
	if(!isConnected)
	{
		ui->statusBarApp->showMessage(tr("Not connected"));
		return false;
	}
	//check if text is not empty
	if(ui->plainTextEditTx->toPlainText().isEmpty())
	{
		ui->statusBarApp->showMessage(tr("No data to send!"));
		return false;
	}
	//if in hexa mode check if text is in hexa
	if(ui->comboBoxTypeTx->currentText()=="toHex" && ui->plainTextEditTx->toPlainText().indexOf(QRegExp("^([0-9a-fA-F]{2})*$"))==-1)
	{
		//and check if text is multiple of 2
		if(ui->plainTextEditTx->toPlainText().size()%2)
			ui->statusBarApp->showMessage(tr("Not multiple of 2!"));
		else
			ui->statusBarApp->showMessage(tr("The text in not in hexa!"));
		return false;
	}
	QByteArray block;
	if(ui->comboBoxTypeTx->currentText()=="Direct") //load the data in plain text mode
	{
		block=ui->plainTextEditTx->toPlainText().toAscii();
		qDebug() << "Data send:" << QString(block);
	}
	else if(ui->comboBoxTypeTx->currentText()=="toHex") //load the data in hexa mode
	{
		block=QByteArray::fromHex(ui->plainTextEditTx->toPlainText().toAscii());
		qDebug() << "Data send in hexa:" << block.toHex();
	}
	if(block.size()==0)
	{
		QMessageBox::information(this,"Error","No text detected!");
		return false;
	}
	QByteArray chunk;
	do
	{
		if(block.size()<=ui->chunkSize->value())
			chunk=block;
		else
			chunk=block.mid(0,ui->chunkSize->value());
		block.remove(0,chunk.size());
		if(ui->compressedStream->isChecked())
			chunk=compress(chunk);
		qDebug() << QString(block.toHex());
		int byteWriten=-2;
		QString ErrorQString;
		if(connectedMode=="Tcp Socket") // if connection is on tcp socket mode
		{
			if(tcpSocket->isWritable() && tcpSocket->isOpen()) //send only if connexion is open
			{
				byteWriten=tcpSocket->write(chunk);
				if(!tcpSocket->waitForBytesWritten(5000) || byteWriten<=0)
					ErrorQString=tcpSocket->errorString();
			}
			else
				ErrorQString=tr("The tcp socket is not open");
		}
		else if(connectedMode=="Local Socket") // if connection is on local socket mode
		{
			if(localSocket->isWritable() && localSocket->isOpen()) //send only if connexion is open
			{
				byteWriten=localSocket->write(chunk);
				if(!localSocket->waitForBytesWritten(5000) || byteWriten<=0)
					ErrorQString=localSocket->errorString();
			}
			else
			{
				if(localSocket->isWritable())
					ErrorQString=tr("The unix socket/named pipe is not open");
				else
					ErrorQString=tr("The unix socket/named pipe is not writable");
			}
		}
		#ifdef RS232_USE
		else if(connectedMode=="RS232") // if connection is on com rs232 mode
		{
			if(port->isWritable() && port->isOpen()) //send only if port is open
			{
				byteWriten=port->write(chunk);
				if(byteWriten<=0)
					ErrorQString=port->errorString();
			}
			else
				ErrorQString=tr("The port com is not open");
		}
		#endif // RS232_USE
		if(byteWriten!=chunk.size()) //check if all the byte have been writen
		{
			if(byteWriten==-1)
				QMessageBox::warning(this,"Error",tr("Byte to write: ")+QString::number(chunk.size())+"\n"+tr("Byte realy writen: ")+QString::number(byteWriten)+"\n"+tr("Error message: ")+ErrorQString);
			else if(byteWriten==0)
			{
				if(ErrorQString!="")
					QMessageBox::warning(this,"Error",tr("No byte writen.")+"\n"+ErrorQString);
				else
					QMessageBox::warning(this,"Error",tr("No byte writen."));
			}
			else
			{
				if(ErrorQString!="")
				{
					QMessageBox::warning(this,"Error",tr("Only ")+QString::number(byteWriten)+tr(" bytes writen!")+"\n"+ErrorQString);
					ui->statusBarApp->showMessage(tr("Only ")+QString::number(byteWriten)+tr(" bytes writen!")+" "+tr("error:")+" "+ErrorQString);
				}
				else
				{
					QMessageBox::warning(this,"Error",tr("Only ")+QString::number(byteWriten)+tr(" bytes writen!"));
					ui->statusBarApp->showMessage(tr("Only ")+QString::number(byteWriten)+tr(" bytes writen!"));
				}
			}
		}
		else
		{
			ui->statusBarApp->showMessage(tr("Bytes send: ")+QString::number(byteWriten));
			return true;
		}
	} while(block.size()>0);
	return false;
}

/** \brief try connect, disconnect if needed
\return Return true if is connected */
bool MainWindow::Connect()
{
	//if already connected try disconnect
	if(isConnected || tcpSocket->state()!=QAbstractSocket::UnconnectedState || localSocket->state()!=QLocalSocket::UnconnectedState)
	{
		qDebug() << "try disconnected before connect";
		if(Disconnect())
		{
			ui->statusBarApp->showMessage(tr("Unable to disconnect"));
			return false;
		}
		if(isConnected || tcpSocket->state()!=QAbstractSocket::UnconnectedState || localSocket->state()!=QLocalSocket::UnconnectedState)
		{
			ui->statusBarApp->showMessage(tr("Unable to disconnect"));
			return false;
		}
	}
	isConnected=false;
	if(ui->comboBoxMode->currentText()=="Tcp Socket") // if connection is tcp socket mode
	{
		if(ui->lineEditIp->text().isEmpty()) // check if ip string is not empty
		{
			ui->statusBarApp->showMessage(tr("Server string is empty!"));
			return false;
		}
		else
		{
			//all ok try connection
			connectedMode="Tcp Socket";
			ui->statusBarApp->showMessage(tr("Connecting..."));
			tcpSocket->connectToHost(ui->lineEditIp->text(),ui->spinBoxIpPort->value(),QIODevice::ReadWrite);
			updateConnectButton();
		}
	}
	else if(ui->comboBoxMode->currentText()=="Local Socket") // if connection is local socket mode
	{
		if(ui->lineEditLocalSocket->text().isEmpty()) //check if socket string is not empty
		{
			ui->statusBarApp->showMessage(tr("Socket name is empty!"));
			return false;
		}
		else
		{
			//all ok try connection
			connectedMode="Local Socket";
			ui->statusBarApp->showMessage(tr("Connecting..."));
			localSocket->connectToServer(ui->lineEditLocalSocket->text(),QIODevice::ReadWrite);
			updateConnectButton();
		}
	}
	#ifdef RS232_USE
	else if(comboBoxMode->currentText()=="RS232") // if connection is on com rs232 mode
	{
		if(comboBoxPort->count()==0)
			statusBarApp->showMessage(tr("No port COM found in system!"));
		else
		{
			//configure the rs232 port
			port->setBaudRate((BaudRateType)comboBoxSpeed->currentIndex());
			port->setFlowControl((FlowType)comboBoxFlow->currentIndex());
			port->setParity((ParityType)comboBoxParity->currentIndex());
			port->setStopBits((StopBitsType)comboBoxStopBit->currentIndex());
			port->setPortName(ports.at(comboBoxPort->currentIndex()).portName);
			qDebug() << "Port try to open: "+ports.at(comboBoxPort->currentIndex()).portName;
			//try open connection port
			if(port->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
			{
				//all is connected
				connectedMode="RS232";
				statusBarApp->showMessage(tr("Connected on port RS232"));
				isConnected=true;
				updateConnectButton();
				#ifndef RS232MODE_EVENT
				pollingTimer->start();	//start polling
				#endif // RS232MODE_EVENT
				updatePortList();
				return true;
			}
			#ifndef Q_OS_WIN32 // if not windows
			if(QFile::exists(ports.at(comboBoxPort->currentIndex()).portName))
				statusBarApp->showMessage(tr("Port RS232 not open: ")+port->errorString());
			else
				statusBarApp->showMessage(tr("Port RS232 not open: Port not found!"));
			#else
				statusBarApp->showMessage(tr("Port RS232 not open: ")+port->errorString());
			#endif // Q_OS_WIN32
		}
		updateConnectButton();
		updatePortList();
		return false;
	}
	#endif // RS232_USE
	return false;
}

/** \brief call this function when new data is incomming
\return Return true if query have successfull loaded */
bool MainWindow::newDataIncomming()
{
	qDebug() << "New data arrived";
	QByteArray block;
	if(tcpSocket->bytesAvailable()) // if connection is on tcp socket mode
		block=tcpSocket->readAll();
	if(localSocket->bytesAvailable()) // if connection is on local socket mode
		QByteArray block=localSocket->readAll();
	#ifdef RS232_USE
	#ifdef RS232MODE_EVENT
	if(port->bytesAvailable()) // if connection is on com rs232 mode
	#else
	if(RS232data.size()) // if connection is on com rs232 mode
	#endif
	{
		#ifdef RS232MODE_EVENT
		QByteArray block=port->readAll();
		#else
		QByteArray block=RS232data;
		RS232data.clear();
		#endif
	}
	#endif // RS232_USE
	if(ui->comboBoxModeAppend->currentText()=="Update")
		IncomingData.clear();
	QByteArray chunk;
	do
	{
		if(block.size()<=ui->chunkSize->value())
			chunk=block;
		else
			chunk=block.mid(0,ui->chunkSize->value());
		block.remove(0,chunk.size());
		if(ui->compressedStream->isChecked())
			chunk=decompress(chunk);
		//load in hexa if needed and append or update the text
		IncomingData.append(chunk);
		//load in hex or not
		if(ui->comboBoxTypeRx->currentText()=="toHex")
			ui->plainTextEditRx->setPlainText(IncomingData.toHex());
		else
			ui->plainTextEditRx->setPlainText(IncomingData);
	} while(block.size()>0);
	return true;

}

/** \brief call this function when error on tcpSocket append
\return always return true and update informations displayed */
bool MainWindow::errorUI(QAbstractSocket::SocketError theErrorDefine)
{
	qDebug() << "errorUI():" << theErrorDefine;
	if(theErrorDefine==QAbstractSocket::RemoteHostClosedError)
		ui->statusBarApp->showMessage(tr("Serveur have closed the connexion"));
	else if(theErrorDefine==QAbstractSocket::ConnectionRefusedError)
		ui->statusBarApp->showMessage(tr("The connection was refused by the peer (or timed out)."));
	else
		ui->statusBarApp->showMessage(tr("Error incomming: ")+QString::number(theErrorDefine));
	isConnected=(tcpSocket->state()==QAbstractSocket::ConnectedState);
	updateConnectButton();
	return true;
}

/** \brief call this function when error on localSocket append
\return always return true and update informations displayed */
bool MainWindow::errorUI(QLocalSocket::LocalSocketError theErrorDefine)
{
	qDebug() << "errorUI():" << theErrorDefine;
	if(theErrorDefine==QLocalSocket::PeerClosedError)
		ui->statusBarApp->showMessage(tr("Serveur have closed the connexion"));
	else if(theErrorDefine==QLocalSocket::ConnectionRefusedError)
		ui->statusBarApp->showMessage(tr("The connection was refused by the peer (or timed out)."));
	else if(theErrorDefine==QLocalSocket::ServerNotFoundError)
		ui->statusBarApp->showMessage(tr("The local socket name was not found."));
	else
		ui->statusBarApp->showMessage(tr("Error incomming: ")+QString::number(theErrorDefine));
	isConnected=(localSocket->state()==QLocalSocket::ConnectedState);
	updateConnectButton();
	return true;
}

/** \brief serveur have close the connection
\return Return true if is diconnected */
bool MainWindow::deconnectClientUI()
{
	isConnected=(tcpSocket->state()==QAbstractSocket::ConnectedState);
	updateConnectButton();
	return !isConnected;
}

/// \brief call this function when connection button is clicked
void MainWindow::ActionClickedConnectButton()
{
	if(isConnected)
	{
		Disconnect();
		updateConnectButton();
	}
	else
	{
		Connect();
		updateConnectButton();
	}
}

/// \brief update the connexion button and information displayed on it
void MainWindow::updateConnectButton()
{
	if(isConnected)
	{
		switch(ui->compressionType->currentIndex())
		{
		case 2:
			compressor=new QtIOCompressor(buffer_compression,9,ui->chunkSize->value());
			compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			compressor->open(QIODevice::WriteOnly);

			decompressor=new QtIOCompressor(buffer_decompression);
			decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			decompressor->open(QIODevice::ReadOnly);
			break;
		case 3:
			compressor=new QtIOCompressor(buffer_compression,9,ui->chunkSize->value());
			compressor->setStreamFormat(QtIOCompressor::GzipFormat);
			compressor->open(QIODevice::WriteOnly);

			decompressor=new QtIOCompressor(buffer_decompression);
			decompressor->setStreamFormat(QtIOCompressor::GzipFormat);
			decompressor->open(QIODevice::ReadOnly);
			break;
		case 0:
		case 1:
		default:
		break;
		}
		ui->pushButtonConnect->setIcon(QIcon(":/images/network-connect.png"));
		ui->pushButtonConnect->setText(tr("Disconnect"));
		ui->compressedStream->setEnabled(false);
		ui->compressionType->setEnabled(false);
		ui->chunkSize->setEnabled(false);
	}
	else
	{
		if(tcpSocket->state()!=QAbstractSocket::UnconnectedState || localSocket->state()!=QLocalSocket::UnconnectedState)
		{
			buffer_decompression_out.resize(0);
			ui->compressedStream->setEnabled(false);
			ui->compressionType->setEnabled(false);
			ui->chunkSize->setEnabled(false);
			ui->pushButtonConnect->setIcon(QIcon(":/images/stop.png"));
			ui->pushButtonConnect->setText(tr("Stop connecting"));
		}
		else
		{
			if(compressor!=NULL)
			{
				compressor->close();
				delete compressor;
				compressor=NULL;
			}
			if(decompressor!=NULL)
			{
				decompressor->close();
				delete decompressor;
				decompressor=NULL;
			}
			ui->compressedStream->setEnabled(true);
			ui->compressionType->setEnabled(true);
			ui->chunkSize->setEnabled(true);
			ui->pushButtonConnect->setIcon(QIcon(":/images/connect.png"));
			ui->pushButtonConnect->setText(tr("Connect"));
		}
	}
}

/// \brief look the mode update information and widget displayed
void MainWindow::UpdateMode()
{
	if(ui->comboBoxMode->currentText()=="Tcp Socket")
	{
		ui->groupBoxNetwork->setVisible(true);
		ui->groupBoxRS232->setVisible(false);
		ui->groupBoxLocalSocket->setVisible(false);
	}
	#ifdef RS232_USE
	else if(ui->comboBoxMode->currentText()=="RS232")
	{
		ui->updatePortList();
		ui->groupBoxNetwork->setVisible(false);
		ui->groupBoxRS232->setVisible(true);
		ui->groupBoxLocalSocket->setVisible(false);
	}
	#endif // RS232_USE
	else if(ui->comboBoxMode->currentText()=="Local Socket")
	{
		ui->groupBoxNetwork->setVisible(false);
		ui->groupBoxRS232->setVisible(false);
		ui->groupBoxLocalSocket->setVisible(true);
	}
}

//mode Rx have changed
bool MainWindow::ModeRxHaveChanged(QString newMode)
{
		//load in hex or not
		if(newMode=="toHex")
			ui->plainTextEditRx->setPlainText(IncomingData.toHex());
		else
			ui->plainTextEditRx->setPlainText(IncomingData);
		return true;
}

//mode Tx have changed
bool MainWindow::ModeTxHaveChanged(QString newMode)
{
	disconnect(ui->plainTextEditTx,SIGNAL(textChanged()),this,SLOT(UpdateTheTxField()));
	//load in hex or not
	if(LastEditionMode=="toHex")
	{
		if(newMode=="Direct")
		{
			OutgoingData=QByteArray::fromHex(ui->plainTextEditTx->toPlainText().toAscii());
			ui->plainTextEditTx->setPlainText(OutgoingData);
			ui->statusBarApp->showMessage("Direct->Hex could be wrong, don't edit for restore the right hex view",10000);
		}
		else
			ui->plainTextEditTx->setPlainText(OutgoingData.toHex());
	}
	else
	{
		if(newMode=="toHex")
			ui->plainTextEditTx->setPlainText(ui->plainTextEditTx->toPlainText().toAscii().toHex());
		else
			ui->plainTextEditTx->setPlainText(QByteArray::fromHex(ui->plainTextEditTx->toPlainText().toAscii()));
	}
	connect(ui->plainTextEditTx,SIGNAL(textChanged()),this,SLOT(UpdateTheTxField()));
	return true;
}

//clear the Rx field
void MainWindow::ClearTheRxField()
{
	IncomingData.clear();
}

//clear the Tx field
void MainWindow::ClearTheTxField()
{
	OutgoingData.clear();
}

//update the Tx field
void MainWindow::UpdateTheTxField()
{
	LastEditionMode=ui->comboBoxTypeTx->currentText();
}

#ifdef RS232_USE

#ifndef RS232MODE_EVENT
void MainWindow::lookNewDataRS232()
{
	if(port->bytesAvailable()>0)
	{
		RS232data=port->readAll();
		newDataIncomming();
	}
}
#endif // RS232MODE_EVENT

//update port list
void MainWindow::updatePortList()
{
	int currentIndex=comboBoxPort->currentIndex();
	comboBoxPort->clear();
	ports = QextSerialEnumerator::getPorts();
	for (int i = 0; i < ports.size(); ++i) {
		comboBoxPort->addItem(ports.at(i).friendName);
	}
	if(currentIndex>=comboBoxPort->count())
		currentIndex=comboBoxPort->count()-1;
	if(currentIndex>=0)
		comboBoxPort->setCurrentIndex(currentIndex);
}

#endif // RS232_USE

//load the Tx file
void MainWindow::loadTheFileTx()
{
	//load file here
	QString file = QFileDialog::getOpenFileName(
		this,
		tr("Select one file to open"),
		"~/",
		tr("All files")+" (*)");
	QFile fileDes(file);
	if(file=="")
	{
		ui->statusBarApp->showMessage("Canceled!",10000);
		return;
	}
	ui->comboBoxTypeTx->setCurrentIndex(1);
	ModeTxHaveChanged("toHex");
	if(fileDes.open(QIODevice::ReadOnly))
	{
		ui->plainTextEditTx->setPlainText(fileDes.readAll().toHex());
		fileDes.close();
	}
	else
		ui->statusBarApp->showMessage("Unabled to open the file in read!",10000);
	UpdateTheTxField();
}

//save the Rx file
void MainWindow::saveTheFileRx()
{
	//save file here
	QString file = QFileDialog::getSaveFileName(
		this,
		tr("Select one file to save"),
		"~/",
		tr("All files")+" (*)");
	QFile fileDes(file);
	if(file=="")
	{
		ui->statusBarApp->showMessage("Canceled!",10000);
		return;
	}
	if(fileDes.open(QIODevice::WriteOnly))
	{
		fileDes.write(IncomingData);
		fileDes.close();
	}
	else
		ui->statusBarApp->showMessage("Unabled to open the file in write!",10000);
}

QByteArray MainWindow::decompress(QByteArray compressed_data)
{
	QByteArray raw_data,chunk_data;
	int returnSize;
	int successLoop;
	switch(ui->compressionType->currentIndex())
	{
	case 2:
	case 3:
		buffer_decompression->seek(0);
		buffer_decompression_out=compressed_data;
		raw_data=decompressor->readAll();
		if(raw_data.size()>0)
		{
			buffer_decompression->seek(0);
			buffer_decompression_out.resize(0);
			ui->statusBarApp->showMessage(tr("Decompressed data: %1").arg(raw_data.size()));
			return raw_data;
		}
		else
		{
			ui->statusBarApp->showMessage(tr("Error at decompressing: %1").arg(decompressor->errorString()));
			return compressed_data;
		}
		break;
	case 0:
	case 1:
		chunk_data.resize(ui->chunkSize->value());
		buffer_decompression_out.append(compressed_data);
		successLoop=0;
		do
		{
			returnSize=QLZ4_uncompress_unknownOutputSize(&buffer_decompression_out,&chunk_data,ui->chunkSize->value());
			if(returnSize==0)
				break;
			if(returnSize<=0)
			{
				if(buffer_decompression_out.size()>ui->chunkSize->value())
				{
					ui->statusBarApp->showMessage(tr("Error at decompressing: %1").arg(returnSize));
					return compressed_data;
				}
				else
					break;
			}
			chunk_data.resize(returnSize);
			raw_data+=chunk_data;
			successLoop++;
		} while(returnSize>0 && buffer_decompression_out.size()>0);
		if(buffer_decompression_out.size()>0)
			ui->statusBarApp->showMessage(tr("block decompressed: %1, remaining compressed data: %2").arg(successLoop).arg(buffer_decompression_out.size()));
		else
			ui->statusBarApp->showMessage(tr("block decompressed: %1").arg(successLoop));
		return raw_data;
		break;
	default:
	break;
	}
	return compressed_data;
}

QByteArray MainWindow::compress(QByteArray raw_data)
{
	QByteArray compressed_data;
	int returnSize;
	switch(ui->compressionType->currentIndex())
	{
	case 2:
	case 3:
		compressor->write(raw_data);
		compressor->flush();//add big overhead
		compressed_data=buffer_compression_out;
		buffer_compression->seek(0);
		buffer_compression_out.resize(0);
		return compressed_data;
		break;
	case 0:
		compressed_data.resize(LZ4_compressBound(raw_data.size()));
		returnSize=LZ4_compressHC(raw_data.constData(),compressed_data.data(),raw_data.size());
		compressed_data.resize(returnSize);
		return compressed_data;
		break;
	case 1:
		compressed_data.resize(LZ4_compressBound(raw_data.size()));
		returnSize=LZ4_compress(raw_data.constData(),compressed_data.data(),raw_data.size());
		compressed_data.resize(returnSize);
		return compressed_data;
		break;
	default:
	break;
	}
	return raw_data;
}

//**************************************
// Constants
//**************************************
#define COPYLENGTH 8
#define ML_BITS 4
#define ML_MASK ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)

//**************************************
// Basic Types
//**************************************
typedef struct _U16_S { quint16 v; } U16_S;
typedef struct _U32_S { quint32 v; } U32_S;
typedef struct _U64_S { quint64 v; } U64_S;

#define A64(x) (((U64_S *)(x))->v)
#define A32(x) (((U32_S *)(x))->v)
#define A16(x) (((U16_S *)(x))->v)

//**************************************
// Architecture-specific macros
//**************************************
#if LZ4_ARCH64	// 64-bit
#define STEPSIZE 8
#define UARCH U64
#define AARCH A64
#define LZ4_COPYSTEP(s,d)		A64(d) = A64(s); d+=8; s+=8;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d)
#define LZ4_SECURECOPY(s,d,e)	if (d<e) LZ4_WILDCOPY(s,d,e)
#define HTYPE U32
#define INITBASE(base)			const quint8* const base = ip
#else		// 32-bit
#define STEPSIZE 4
#define UARCH U32
#define AARCH A32
#define LZ4_COPYSTEP(s,d)		A32(d) = A32(s); d+=4; s+=4;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d); LZ4_COPYSTEP(s,d);
#define LZ4_SECURECOPY			LZ4_WILDCOPY
#define HTYPE const quint8*
#define INITBASE(base)			const int base = 0
#endif

//**************************************
// Macros
//**************************************
#define LZ4_WILDCOPY(s,d,e)		do { LZ4_COPYPACKET(s,d) } while (d<e);

#if (defined(LZ4_BIG_ENDIAN) && !defined(BIG_ENDIAN_NATIVE_BUT_INCOMPATIBLE))
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { U16 v = A16(p); v = bswap16(v); d = (s) - v; }
#define LZ4_WRITE_LITTLEENDIAN_16(p,i) { U16 v = (U16)(i); v = bswap16(v); A16(p) = v; p+=2; }
#else		// Little Endian
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { d = (s) - A16(p); }
#define LZ4_WRITE_LITTLEENDIAN_16(p,v) { A16(p) = v; p+=2; }
#endif

int QLZ4_uncompress_unknownOutputSize(QByteArray *source,
				QByteArray *destination,
				int maxOutputSize)
{
	// Local Variables
	const quint8* ip = (const quint8*) source->data();
	const quint8* const iend = ip + source->size();
	const quint8* ref;

	quint8* op = (quint8*) destination->data();
	quint8* const oend = op + maxOutputSize;
	quint8* cpy;

	size_t dec[] ={0, 3, 2, 3, 0, 0, 0, 0};

	quint8 token;
	int length;

	// Main Loop
	while (ip<iend)
	{
		// get runlength
		token = *ip++;
		if ((length=(token>>ML_BITS)) == RUN_MASK) { int s=255; while ((ip<iend) && (s==255)) { s=*ip++; length += s; } }

		// copy literals
		cpy = op+length;
		if ((cpy>oend-COPYLENGTH) || (ip+length>iend-COPYLENGTH))
		{
			if (cpy > oend)
				return -1;
			if (ip+length > iend)
				return -2;
			memcpy(op, ip, length);
			op += length;
			ip += length;
			//the remaining copy is of other packet
			/*if (ip<iend)
				return -3;*/
			break;    // Necessarily EOF, due to parsing restrictions
		}
		do { LZ4_COPYPACKET(ip,op) } while (op<cpy);
		ip -= (op-cpy); op = cpy;

		// get offset
		LZ4_READ_LITTLEENDIAN_16(ref,cpy,ip); ip+=2;
		if (ref < (quint8* const)destination->data())
			return -4;

		// get matchlength
		if ((length=(token&ML_MASK)) == ML_MASK) { while (ip<iend) { int s = *ip++; length +=s; if (s==255) continue; break; } }

		// copy repeated sequence
		if(__builtin_expect (((op-ref<STEPSIZE) != 0),0))
				//(expect(, 0))
		{
#if LZ4_ARCH64
			size_t dec2table[]={0, 0, 0, -1, 0, 1, 2, 3};
			size_t dec2 = dec2table[op-ref];
#else
			const int dec2 = 0;
#endif
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			ref -= dec[op-ref];
			A32(op)=A32(ref); op += STEPSIZE-4;
			ref -= dec2;
		} else { LZ4_COPYSTEP(ref,op); }
		cpy = op + length - (STEPSIZE-4);
		if (cpy>oend-COPYLENGTH)
		{
			if (cpy > oend)
				return -5;
			LZ4_SECURECOPY(ref, op, (oend-COPYLENGTH));
			while(op<cpy) *op++=*ref++;
			op=cpy;
			if (op == oend) break;    // Check EOF (should never happen, since last 5 bytes are supposed to be literals)
			continue;
		}
		LZ4_SECURECOPY(ref, op, cpy);
		op=cpy;		// correction
	}

	// end of decoding
	destination->resize((int)(((char*)op)-destination->data()));
	source->remove(0,(int)(((char*)ip)-source->data()));
	return destination->size();
}
