#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#if defined (Q_OS_WIN32) || defined (Q_OS_WINCE)
#define COMBOBOXTEXTLOCALSOCK "Named pipe (QLocalServer)"
#else
#define COMBOBOXTEXTLOCALSOCK "Unix socket (QLocalServer)"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->comboBoxTypeTx->setCurrentIndex(1);
	ui->comboBoxTypeRx->setCurrentIndex(1);
	connect(ui->comboBoxMode,SIGNAL(currentIndexChanged(int)),this,SLOT(updateMode()));
	connect(&TcpServer,SIGNAL(newConnection()),this,SLOT(newClient()));
	connect(&LocalServer,SIGNAL(newConnection()),this,SLOT(newClient()));
	isConnected=false;
	idConnection=1;
	ui->comboBoxMode->setItemText(0,"Tcp socket (QTcpServer)");
	ui->comboBoxMode->setItemText(1,COMBOBOXTEXTLOCALSOCK);
	updateMode();
}

int MainWindow::indexOfSelection()
{
	if(connectedMode=="Tcp socket (QTcpServer)")
	for (int i = 0; i < tcpSocket.size(); ++i) {
		if(tcpSocket.at(i)->ItemInList->isSelected())
			return i;
	}
	if(connectedMode==COMBOBOXTEXTLOCALSOCK)
	for (int i = 0; i < localSocket.size(); ++i) {
		if(localSocket.at(i)->ItemInList->isSelected())
			return i;
	}
	return -1;
}

void MainWindow::newClient()
{
	if(connectedMode=="Tcp socket (QTcpServer)")
	{
		QTcpSocket *newClient = TcpServer.nextPendingConnection();
		ui->statusBarApp->showMessage("New client connected: "+newClient->peerAddress().toString(),10000);
		//connect(newClient, SIGNAL(readyRead()), this, SLOT(dataincommingUI()));
		connect(newClient, SIGNAL(disconnected()), this, SLOT(clientLeave()));
		connect(newClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(newErrorTcp(QAbstractSocket::SocketError)));
		connect(newClient, SIGNAL(readyRead()), this, SLOT(newData()));
		tcpSocket << new tcpClientInfo;
		tcpSocket.last()->tcpSocket=newClient;
		tcpSocket.last()->isConnected=true;
		tcpSocket.last()->LastEditionMode=ui->comboBoxTypeTx->currentText();
		QListWidgetItem *TempItem=new QListWidgetItem(QIcon(":/images/preferences-desktop-user.png"),QString::number(idConnection++)+": "+newClient->peerAddress().toString());
		TempItem->setToolTip("ip: "+newClient->peerAddress().toString()+", port: "+QString::number(newClient->peerPort()));
		tcpSocket.last()->ItemInList=TempItem;
		ui->listWidgetClient->addItem(TempItem);

		//compression part
		tcpSocket.last()->buffer_compression=new QBuffer(&tcpSocket.last()->buffer_compression_out);
		tcpSocket.last()->compressor=NULL;
		tcpSocket.last()->buffer_decompression=new QBuffer(&tcpSocket.last()->buffer_decompression_out);
		tcpSocket.last()->decompressor=NULL;
		switch(ui->compressionType->currentIndex())
		{
		case 2:
			tcpSocket.last()->compressor=new QtIOCompressor(tcpSocket.last()->buffer_compression,9,ui->chunkSize->value());
			tcpSocket.last()->compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			tcpSocket.last()->compressor->open(QIODevice::WriteOnly);

			tcpSocket.last()->decompressor=new QtIOCompressor(tcpSocket.last()->buffer_decompression);
			tcpSocket.last()->decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			tcpSocket.last()->decompressor->open(QIODevice::ReadOnly);
			break;
		case 3:
			tcpSocket.last()->compressor=new QtIOCompressor(tcpSocket.last()->buffer_compression,9,ui->chunkSize->value());
			tcpSocket.last()->compressor->setStreamFormat(QtIOCompressor::GzipFormat);
			tcpSocket.last()->compressor->open(QIODevice::WriteOnly);

			tcpSocket.last()->decompressor=new QtIOCompressor(tcpSocket.last()->buffer_decompression);
			tcpSocket.last()->decompressor->setStreamFormat(QtIOCompressor::GzipFormat);
			tcpSocket.last()->decompressor->open(QIODevice::ReadOnly);
			break;
		case 0:
		case 1:
		default:
		break;
		}

	}
	if(connectedMode==COMBOBOXTEXTLOCALSOCK)
	{
		QLocalSocket *newClient = LocalServer.nextPendingConnection();
		ui->statusBarApp->showMessage("New client connected",10000);
		connect(newClient, SIGNAL(disconnected()), this, SLOT(clientLeave()));
		connect(newClient, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(newErrorLocal(QLocalSocket::LocalSocketError)));
		connect(newClient, SIGNAL(readyRead()), this, SLOT(newData()));
		localSocket << new localClientInfo;
		localSocket.last()->localSocket=newClient;
		localSocket.last()->isConnected=true;
		localSocket.last()->LastEditionMode=ui->comboBoxTypeTx->currentText();
		localSocket.last()->ItemInList=new QListWidgetItem(QIcon(":/images/preferences-desktop-user.png"),QString::number(idConnection++)+": Local Client");
		ui->listWidgetClient->addItem(localSocket.last()->ItemInList);

		//compression part
		localSocket.last()->buffer_compression=new QBuffer(&tcpSocket.last()->buffer_compression_out);
		localSocket.last()->compressor=NULL;
		localSocket.last()->buffer_decompression=new QBuffer(&tcpSocket.last()->buffer_decompression_out);
		localSocket.last()->decompressor=NULL;
		switch(ui->compressionType->currentIndex())
		{
		case 2:
			localSocket.last()->compressor=new QtIOCompressor(tcpSocket.last()->buffer_compression,9,ui->chunkSize->value());
			localSocket.last()->compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			localSocket.last()->compressor->open(QIODevice::WriteOnly);

			localSocket.last()->decompressor=new QtIOCompressor(tcpSocket.last()->buffer_decompression);
			localSocket.last()->decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			localSocket.last()->decompressor->open(QIODevice::ReadOnly);
			break;
		case 3:
			localSocket.last()->compressor=new QtIOCompressor(tcpSocket.last()->buffer_compression,9,ui->chunkSize->value());
			localSocket.last()->compressor->setStreamFormat(QtIOCompressor::GzipFormat);
			localSocket.last()->compressor->open(QIODevice::WriteOnly);

			localSocket.last()->decompressor=new QtIOCompressor(tcpSocket.last()->buffer_decompression);
			localSocket.last()->decompressor->setStreamFormat(QtIOCompressor::GzipFormat);
			localSocket.last()->decompressor->open(QIODevice::ReadOnly);
			break;
		case 0:
		case 1:
		default:
		break;
		}
	}
	if(ui->listWidgetClient->count()==1)
	{
		ui->listWidgetClient->item(0)->setSelected(true);
		updateCurrentData();
	}
}

void MainWindow::newData()
{
	if(connectedMode=="Tcp socket (QTcpServer)") // if connection is on tcp socket mode
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
		int index=-1;
		for (int i = 0; i < tcpSocket.size(); ++i) {
			if(tcpSocket.at(i)->tcpSocket==socket)
				index=i;
		}
		if(index==-1)
		{
			ui->statusBarApp->showMessage("Internal error at newData()",10000);
			return;
		}
		QByteArray block=socket->readAll();
		//load in hexa if needed and append or update the text
		if(ui->comboBoxModeAppend->currentText()=="Update")
			tcpSocket[index]->IncomingData.clear();

		QByteArray chunk;
		do
		{
			if(block.size()<=ui->chunkSize->value())
				chunk=block;
			else
				chunk=block.mid(0,ui->chunkSize->value());
			block.remove(0,chunk.size());
			if(ui->compressedStream->isChecked())
				chunk=decompress(chunk,tcpSocket[index]->buffer_decompression,&tcpSocket[index]->buffer_decompression_out,tcpSocket[index]->compressor);
			//load in hexa if needed and append or update the text
			tcpSocket[index]->IncomingData.append(chunk);
		} while(block.size()>0);


		updateCurrentData();
	}
	if(connectedMode==COMBOBOXTEXTLOCALSOCK) // if connection is on local socket mode
	{
		QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
		int index=-1;
		for (int i = 0; i < localSocket.size(); ++i) {
			if(localSocket.at(i)->localSocket==socket)
				index=i;
		}
		if(index==-1)
		{
			ui->statusBarApp->showMessage("Internal error at newData()",10000);
			return;
		}
		QByteArray block=socket->readAll();
		//load in hexa if needed and append or update the text
		if(ui->comboBoxModeAppend->currentText()=="Update")
			localSocket[index]->IncomingData.clear();

		QByteArray chunk;
		do
		{
			if(block.size()<=ui->chunkSize->value())
				chunk=block;
			else
				chunk=block.mid(0,ui->chunkSize->value());
			block.remove(0,chunk.size());
			if(ui->compressedStream->isChecked())
				chunk=decompress(chunk,localSocket[index]->buffer_decompression,&localSocket[index]->buffer_decompression_out,localSocket[index]->compressor);
			//load in hexa if needed and append or update the text
			localSocket[index]->IncomingData.append(chunk);
		} while(block.size()>0);

		localSocket[index]->IncomingData.append(block);
		updateCurrentData();
	}
}

void MainWindow::updateCurrentData()
{
	if(indexOfSelection()!=-1)
	{
		if(connectedMode=="Tcp socket (QTcpServer)") // if connection is on tcp socket mode
		{
			//load in hex or not
			if(ui->comboBoxTypeRx->currentText()=="toHex")
				ui->textEditFromClient->setPlainText(tcpSocket.at(indexOfSelection())->IncomingData.toHex());
			else
				ui->textEditFromClient->setPlainText(tcpSocket.at(indexOfSelection())->IncomingData);
		}
		if(connectedMode==COMBOBOXTEXTLOCALSOCK) // if connection is on local socket mode
		{
			//load in hex or not
			if(ui->comboBoxTypeRx->currentText()=="toHex")
				ui->textEditFromClient->setPlainText(localSocket.at(indexOfSelection())->IncomingData.toHex());
			else
				ui->textEditFromClient->setPlainText(localSocket.at(indexOfSelection())->IncomingData);
		}
	}
	else
	{
		ui->textEditToClient->clear();
		ui->textEditFromClient->clear();
	}
}

void MainWindow::clientLeave()
{
	if(connectedMode=="Tcp socket (QTcpServer)")
	{
		// Wich client leave
		QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
		if (socket == 0) // If not found
			return;
		int index=-1;
		for (int i = 0; i < tcpSocket.size(); ++i) {
			if(tcpSocket.at(i)->tcpSocket==socket)
				index=i;
		}
		if(index==-1)
			ui->statusBarApp->showMessage("Unable to locate client, internal error",10000);
		else
		{
			if(TcpServer.isListening())
				ui->statusBarApp->showMessage("The client: "+socket->peerAddress().toString()+" as leave",10000);
			tcpSocket[index]->ItemInList->setIcon(QIcon(":/images/list-disconnected.png"));
			tcpSocket[index]->isConnected=false;
			tcpSocket[index]->tcpSocket=NULL;
			socket->deleteLater();

			//compression
			if(tcpSocket[index]->compressor!=NULL)
			{
				tcpSocket[index]->compressor->close();
				delete tcpSocket[index]->compressor;
				tcpSocket[index]->compressor=NULL;
			}
			if(tcpSocket[index]->decompressor!=NULL)
			{
				tcpSocket[index]->decompressor->close();
				delete tcpSocket[index]->decompressor;
				tcpSocket[index]->decompressor=NULL;
			}
		}
	}
	if(connectedMode==COMBOBOXTEXTLOCALSOCK)
	{
		// Wich client leave
		QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
		if (socket == 0) // If not found
			return;
		int index=-1;
		for (int i = 0; i < localSocket.size(); ++i) {
			if(localSocket.at(i)->localSocket==socket)
				index=i;
		}
		if(index==-1)
			ui->statusBarApp->showMessage("Unable to locate client, internal error",10000);
		else
		{
			if(LocalServer.isListening())
				ui->statusBarApp->showMessage("Client as leave",10000);
			localSocket[index]->ItemInList->setIcon(QIcon(":/images/list-disconnected.png"));
			localSocket[index]->isConnected=false;
			localSocket[index]->localSocket=NULL;
			socket->deleteLater();
		}

		//compression
		if(localSocket[index]->compressor!=NULL)
		{
			localSocket[index]->compressor->close();
			delete localSocket[index]->compressor;
			localSocket[index]->compressor=NULL;
		}
		if(localSocket[index]->decompressor!=NULL)
		{
			localSocket[index]->decompressor->close();
			delete localSocket[index]->decompressor;
			localSocket[index]->decompressor=NULL;
		}
	}
}

void MainWindow::newErrorLocal(QLocalSocket::LocalSocketError error)
{
	if(error!=QLocalSocket::PeerClosedError)
	   ui->statusBarApp->showMessage("Error detected ("+QString::number(error),10000);
}

void MainWindow::newErrorTcp(QAbstractSocket::SocketError error)
{
	if(error!=QAbstractSocket::RemoteHostClosedError)
	   ui->statusBarApp->showMessage("Error detected ("+QString::number(error),10000);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::updateMode()
{
	ui->groupBoxLocalSocket->hide();
	ui->groupBoxTcpSocket->hide();
	if(ui->comboBoxMode->currentText()=="Tcp socket (QTcpServer)")
		ui->groupBoxTcpSocket->show();
	if(ui->comboBoxMode->currentText()==COMBOBOXTEXTLOCALSOCK)
		ui->groupBoxLocalSocket->show();
}

void MainWindow::on_LoadFile_clicked()
{
	if(indexOfSelection()==-1)
		ui->statusBarApp->showMessage("Select client before",10000);
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
	on_comboBoxTypeTx_currentIndexChanged("toHex");
	if(fileDes.open(QIODevice::ReadOnly))
	{
		ui->textEditToClient->setPlainText(fileDes.readAll().toHex());
		fileDes.close();
	}
	else
		ui->statusBarApp->showMessage("Unabled to open the file in read!",10000);
	on_textEditToClient_textChanged();
}

//mode Rx have changed
bool MainWindow::ModeRxHaveChanged(QString newMode)
{
	int index=ui->listWidgetClient->currentRow();
	if(index==-1)
	{
		ui->textEditToClient->clear();
		return false;
	}
	else
	{
		//load in hex or not
		if(newMode=="toHex")
			if(connectedMode==COMBOBOXTEXTLOCALSOCK)
				ui->textEditFromClient->setPlainText(localSocket.at(indexOfSelection())->IncomingData.toHex());
			else
				ui->textEditFromClient->setPlainText(tcpSocket.at(indexOfSelection())->IncomingData.toHex());
		else
			if(connectedMode==COMBOBOXTEXTLOCALSOCK)
				ui->textEditFromClient->setPlainText(localSocket.at(indexOfSelection())->IncomingData);
			else
				ui->textEditFromClient->setPlainText(tcpSocket.at(indexOfSelection())->IncomingData);
		return true;
	}
}

void MainWindow::on_SaveFile_clicked()
{
	if(indexOfSelection()==-1)
		ui->statusBarApp->showMessage("Select client before",10000);
	int index=ui->listWidgetClient->currentRow();
	if(index==-1)
	{
		ui->statusBarApp->showMessage("No client selected!",10000);
		return;
	}
	else
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
			fileDes.write(localSocket.at(indexOfSelection())->IncomingData);
			fileDes.close();
		}
		else
			ui->statusBarApp->showMessage("Unabled to open the file in write!",10000);
	}
}

void MainWindow::on_ClearDataToClient_clicked()
{
	if(indexOfSelection()!=-1)
	{
		if(connectedMode=="Tcp socket (QTcpServer)")
			tcpSocket.at(indexOfSelection())->OutgoingData.clear();
		if(connectedMode==COMBOBOXTEXTLOCALSOCK)
			localSocket.at(indexOfSelection())->OutgoingData.clear();
	}
}

void MainWindow::on_ClearDataFromClient_clicked()
{
	if(indexOfSelection()!=-1)
	{
		if(connectedMode=="Tcp socket (QTcpServer)")
			tcpSocket.at(indexOfSelection())->IncomingData.clear();
		if(connectedMode==COMBOBOXTEXTLOCALSOCK)
			localSocket.at(indexOfSelection())->IncomingData.clear();
	}
}

/// \brief update the connexion button and information displayed on it
void MainWindow::updateConnectButton()
{
	if(isConnected)
	{
		ui->pushButtonConnect->setIcon(QIcon(":/images/network-connect.png"));
		ui->pushButtonConnect->setText(tr("Stop listen"));
	}
	else
	{
		ui->pushButtonConnect->setIcon(QIcon(":/images/connect.png"));
		ui->pushButtonConnect->setText(tr("Listen"));
	}
}

/// \brief call this function when connection button is clicked
void MainWindow::on_pushButtonConnect_clicked()
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

/** \brief try disconnect the current connexion
\return Return true if disconnect */
bool MainWindow::Disconnect()
{
	ui->compressedStream->setEnabled(true);
	ui->compressionType->setEnabled(true);
	ui->chunkSize->setEnabled(true);
	if(isConnected)
	{
		for (int i = 0; i < tcpSocket.size(); ++i) {
			if(tcpSocket.at(i)->isConnected)
				tcpSocket.at(i)->tcpSocket->disconnectFromHost();
		}
		for (int i = 0; i < localSocket.size(); ++i) {
			if(localSocket.at(i)->isConnected)
				localSocket.at(i)->localSocket->disconnectFromServer();
		}
		if(connectedMode=="Tcp socket (QTcpServer)")
			TcpServer.close();
		if(connectedMode==COMBOBOXTEXTLOCALSOCK)
			LocalServer.close();
		isConnected=false;
		ui->statusBarApp->showMessage("Server closed",10000);
		ui->listWidgetClient->clear();
		tcpSocket.clear();
		localSocket.clear();
		idConnection=1;
		return true;
	}
	return false;
}

/** \brief try connect, disconnect if needed
\return Return true if is connected */
bool MainWindow::Connect()
{
	if(!isConnected)
	{
		connectedMode=ui->comboBoxMode->currentText();
		if(connectedMode=="Tcp socket (QTcpServer)")
		{
			isConnected=TcpServer.listen(QHostAddress(QHostAddress::Any),ui->port->value());
			if(!isConnected)
				ui->statusBarApp->showMessage("Server failed to listen: "+TcpServer.errorString(),10000);
		}
		else if(connectedMode==COMBOBOXTEXTLOCALSOCK)
		{
			isConnected=LocalServer.listen(ui->lineEditSocket->text());
			if(!isConnected)
				ui->statusBarApp->showMessage("Server failed to listen: "+LocalServer.errorString(),10000);
		}
		else
			ui->statusBarApp->showMessage("Internal error",10000);
		if(isConnected)
		{
			ui->compressedStream->setEnabled(false);
			ui->compressionType->setEnabled(false);
			ui->chunkSize->setEnabled(false);
			ui->statusBarApp->showMessage("Server open and listen connection",10000);
			return true;
		}
	}
	return false;
}

//update the Tx field
void MainWindow::on_textEditToClient_textChanged()
{
	if(indexOfSelection()!=-1)
	{
		if(connectedMode=="Tcp socket (QTcpServer)")
			tcpSocket[indexOfSelection()]->LastEditionMode=ui->comboBoxTypeTx->currentText();
		else
			localSocket[indexOfSelection()]->LastEditionMode=ui->comboBoxTypeTx->currentText();
		//ModeTxHaveChanged(ui->comboBoxTypeTx->currentText());
	}
}

//mode Tx have changed
void MainWindow::on_comboBoxTypeTx_currentIndexChanged(QString newMode)
{
	int index=ui->listWidgetClient->currentRow();
	if(index==-1)
	{
		ui->statusBarApp->showMessage("No client selected!",10000);
		ui->textEditToClient->clear();
	}
	else
	{
		//load in hex or not
		if(connectedMode=="Tcp socket (QTcpServer)")
		{
			if(tcpSocket.at(indexOfSelection())->LastEditionMode=="toHex")
			{
				if(newMode=="Direct")
				{
					tcpSocket[indexOfSelection()]->OutgoingData=QByteArray::fromHex(ui->textEditToClient->toPlainText().toAscii());
					ui->textEditToClient->setPlainText(tcpSocket.at(indexOfSelection())->OutgoingData);
					ui->statusBarApp->showMessage("Direct->Hex could be wrong, don't edit for restore the right hex view",10000);
				}
				else
					ui->textEditToClient->setPlainText(tcpSocket.at(indexOfSelection())->OutgoingData.toHex());
			}
			else
			{
				if(newMode=="toHex")
					ui->textEditToClient->setPlainText(ui->textEditToClient->toPlainText().toAscii().toHex());
				else
					ui->textEditToClient->setPlainText(QByteArray::fromHex(ui->textEditToClient->toPlainText().toAscii()));
			}
		}
	}
}

void MainWindow::on_toolButton_clicked()
{
	if(indexOfSelection()!=-1)
	{
		if(connectedMode=="Tcp socket (QTcpServer)")
			if(tcpSocket.at(indexOfSelection())->isConnected)
				tcpSocket[indexOfSelection()]->tcpSocket->disconnectFromHost();
		if(connectedMode==COMBOBOXTEXTLOCALSOCK)
			if(localSocket.at(indexOfSelection())->isConnected)
				localSocket[indexOfSelection()]->localSocket->disconnectFromServer();
	}
}

void MainWindow::on_listWidgetClient_itemSelectionChanged()
{
	updateCurrentData();
}

void MainWindow::on_comboBoxTypeRx_currentIndexChanged(QString )
{
	updateCurrentData();
}

//mode Tx have changed
bool MainWindow::ModeTxHaveChanged(QString newMode)
{
	//load in hex or not
	QString LastEditionMode;
	QByteArray OutgoingData;
	if(connectedMode=="Tcp socket (QTcpServer)")
	{
		OutgoingData=tcpSocket.at(indexOfSelection())->OutgoingData;
		LastEditionMode=tcpSocket.at(indexOfSelection())->LastEditionMode;
	}
	if(connectedMode==COMBOBOXTEXTLOCALSOCK)
	{
		OutgoingData=localSocket.at(indexOfSelection())->OutgoingData;
		LastEditionMode=localSocket.at(indexOfSelection())->LastEditionMode;
	}
	if(LastEditionMode=="toHex")
	{
		if(newMode=="Direct")
		{
			OutgoingData=QByteArray::fromHex(ui->textEditToClient->toPlainText().toAscii());
			ui->textEditToClient->setPlainText(OutgoingData);
			ui->statusBarApp->showMessage("Direct->Hex could be wrong, don't edit for restore the right hex view",10000);
		}
		else
			ui->textEditToClient->setPlainText(OutgoingData.toHex());
	}
	else
	{
		if(newMode=="toHex")
			ui->textEditToClient->setPlainText(ui->textEditToClient->toPlainText().toAscii().toHex());
		else
			ui->textEditToClient->setPlainText(QByteArray::fromHex(ui->textEditToClient->toPlainText().toAscii()));
	}
	return true;
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
	if(ui->textEditToClient->toPlainText().isEmpty())
	{
		ui->statusBarApp->showMessage(tr("No data to send!"));
		return false;
	}
	//if in hexa mode check if text is in hexa
	if(ui->comboBoxTypeTx->currentText()=="toHex" && ui->textEditToClient->toPlainText().indexOf(QRegExp("^([0-9a-fA-F]{2})*$"))==-1)
	{
		//and check if text is multiple of 2
		if(ui->textEditToClient->toPlainText().size()%2)
			ui->statusBarApp->showMessage(tr("Not multiple of 2!"));
		else
			ui->statusBarApp->showMessage(tr("The text in not in hexa!"));
		return false;
	}
	QByteArray block;
	if(ui->comboBoxTypeTx->currentText()=="Direct") //load the data in plain text mode
	{
		block=ui->textEditToClient->toPlainText().toAscii();
		qDebug() << "Data send:" << QString(block);
	}
	else if(ui->comboBoxTypeTx->currentText()=="toHex") //load the data in hexa mode
	{
		block=QByteArray::fromHex(ui->textEditToClient->toPlainText().toAscii());
		qDebug() << "Data send in hexa:" << block.toHex();
	}
	if(block.size()==0)
	{
		ui->statusBarApp->showMessage(tr("No text detected!"));
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
		int byteWriten=-2;
		QString ErrorQString;
		if(connectedMode=="Tcp socket (QTcpServer)") // if connection is on tcp socket mode
		{
			if(tcpSocket.at(indexOfSelection())->tcpSocket==NULL)
				ErrorQString="Socket drop";
			else if(tcpSocket.at(indexOfSelection())->tcpSocket->state()!=QTcpSocket::ConnectedState)
				ErrorQString="Socket not connected";
			else
			{
				if(ui->compressedStream->isChecked())
					chunk=compress(chunk,tcpSocket.at(indexOfSelection())->buffer_compression,&tcpSocket.at(indexOfSelection())->buffer_compression_out,tcpSocket.at(indexOfSelection())->compressor);
				byteWriten=tcpSocket.at(indexOfSelection())->tcpSocket->write(chunk);
				if(byteWriten<=0)
					ErrorQString=tcpSocket.at(indexOfSelection())->tcpSocket->errorString();
			}
		}
		else if(connectedMode==COMBOBOXTEXTLOCALSOCK) // if connection is on local socket mode
		{
			if(localSocket.at(indexOfSelection())->localSocket==NULL)
				ErrorQString="Socket drop";
			else if(localSocket.at(indexOfSelection())->localSocket->state()!=QLocalSocket::ConnectedState)
				ErrorQString="Socket not connected";
			else
			{
				if(ui->compressedStream->isChecked())
					chunk=compress(chunk,localSocket.at(indexOfSelection())->buffer_compression,&localSocket.at(indexOfSelection())->buffer_compression_out,localSocket.at(indexOfSelection())->compressor);
				byteWriten=localSocket.at(indexOfSelection())->localSocket->write(chunk);
				if(byteWriten<=0)
					ErrorQString=localSocket.at(indexOfSelection())->localSocket->errorString();
			}
		}
		if(byteWriten!=chunk.size()) //check if all the byte have been writen
		{
			if(byteWriten==-1)
				ui->statusBarApp->showMessage(tr("Byte to write: ")+QString::number(chunk.size())+"\n"+tr("Byte realy writen: ")+QString::number(byteWriten)+"\n"+tr("Error message: ")+ErrorQString);
			else if(byteWriten==0)
				ui->statusBarApp->showMessage("No byte writen.");
			else
				ui->statusBarApp->showMessage(tr("Only ")+QString::number(byteWriten)+tr(" bytes writen: ")+ErrorQString);
		}
		else
		{
			ui->statusBarApp->showMessage(tr("Bytes send: ")+QString::number(byteWriten));
			return true;
		}
	} while(block.size()>0);
	return false;
}

void MainWindow::on_pushButtonSend_clicked()
{
	if(indexOfSelection()!=-1)
		sendNewData();
}

QByteArray MainWindow::decompress(QByteArray compressed_data,QBuffer *buffer_decompression,QByteArray *buffer_decompression_out,QtIOCompressor* decompressor)
{
	QByteArray raw_data,chunk_data;
	int returnSize;
	int successLoop;
	switch(ui->compressionType->currentIndex())
	{
	case 2:
	case 3:
		buffer_decompression->seek(0);
		*buffer_decompression_out=compressed_data;
		raw_data=decompressor->readAll();
		if(raw_data.size()>0)
		{
			buffer_decompression->seek(0);
			buffer_decompression_out->resize(0);
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
		buffer_decompression_out->append(compressed_data);
		successLoop=0;
		do
		{
			returnSize=QLZ4_uncompress_unknownOutputSize(buffer_decompression_out,&chunk_data,ui->chunkSize->value());
			if(returnSize==0)
				break;
			if(returnSize<=0)
			{
				if(buffer_decompression_out->size()>ui->chunkSize->value())
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
		} while(returnSize>0 && buffer_decompression_out->size()>0);
		if(buffer_decompression_out->size()>0)
			ui->statusBarApp->showMessage(tr("block decompressed: %1, remaining compressed data: %2").arg(successLoop).arg(buffer_decompression_out->size()));
		else
			ui->statusBarApp->showMessage(tr("block decompressed: %1").arg(successLoop));
		return raw_data;
		break;
	default:
	break;
	}
	return compressed_data;
}

QByteArray MainWindow::compress(QByteArray raw_data,QBuffer *buffer_compression,QByteArray *buffer_compression_out,QtIOCompressor* compressor)
{
	QByteArray compressed_data;
	int returnSize;
	switch(ui->compressionType->currentIndex())
	{
	case 2:
	case 3:
		compressor->write(raw_data);
		compressor->flush();//add big overhead
		compressed_data=*buffer_compression_out;
		buffer_compression->seek(0);
		buffer_compression_out->resize(0);
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


