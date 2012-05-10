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
	on_compressionType_currentIndexChanged(0);
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
		tcpSocket.last()->compression=NULL;
		switch(ui->compressionType->currentIndex())
		{
		case 2:
			tcpSocket.last()->compression=new ZlibCompressionTcpSocket(ui->compressionBufferSize->value(),9,ui->compressionByPacket->isChecked());
			break;
		case 3:
			tcpSocket.last()->compression=new GzipCompressionTcpSocket(ui->compressionBufferSize->value(),9,ui->compressionByPacket->isChecked());
			break;
		case 0:
			tcpSocket.last()->compression=new Lz4HcCompressionTcpSocket();
			break;
		case 1:
		default:
			tcpSocket.last()->compression=new Lz4CompressionTcpSocket();
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
		localSocket.last()->compression=NULL;
		switch(ui->compressionType->currentIndex())
		{
		case 2:
			localSocket.last()->compression=new ZlibCompressionTcpSocket(ui->compressionBufferSize->value(),9,ui->compressionByPacket->isChecked());
			break;
		case 3:
			localSocket.last()->compression=new GzipCompressionTcpSocket(ui->compressionBufferSize->value(),9,ui->compressionByPacket->isChecked());
			break;
		case 0:
			localSocket.last()->compression=new Lz4HcCompressionTcpSocket();
			break;
		case 1:
		default:
			localSocket.last()->compression=new Lz4CompressionTcpSocket();
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

		QByteArray raw_data;
		if(ui->compressedStream->isChecked())
			raw_data=tcpSocket[index]->compression->decompressData(block);
		else
			raw_data=block;
		//load in hexa if needed and append or update the text
		tcpSocket[index]->IncomingData.append(raw_data);

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

		QByteArray raw_data;
		if(ui->compressedStream->isChecked())
			raw_data=tcpSocket[index]->compression->decompressData(block);
		else
			raw_data=block;

		localSocket[index]->IncomingData.append(raw_data);
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
			if(tcpSocket[index]->compression!=NULL)
			{
				delete tcpSocket[index]->compression;
				tcpSocket[index]->compression=NULL;
			}
			if(tcpSocket[index]->compression!=NULL)
			{
				delete tcpSocket[index]->compression;
				tcpSocket[index]->compression=NULL;
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
		if(localSocket[index]->compression!=NULL)
		{
			delete localSocket[index]->compression;
			localSocket[index]->compression=NULL;
		}
		if(localSocket[index]->compression!=NULL)
		{
			delete localSocket[index]->compression;
			localSocket[index]->compression=NULL;
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
	if(fileDes.size()>1024*1024)
	{
		ui->statusBarApp->showMessage("File to big to be read",10000);
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
	if(isConnected)
	{
		ui->compressedStream->setEnabled(true);
		ui->compressionOptions->setEnabled(true);
		on_compressionType_currentIndexChanged(0);
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
			ui->compressionOptions->setEnabled(false);
			ui->compressionType->setEnabled(false);
			ui->compressedStream->setEnabled(false);
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

	QByteArray compressedData=block;
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
				compressedData=tcpSocket.at(indexOfSelection())->compression->compressData(block);
			byteWriten=tcpSocket.at(indexOfSelection())->tcpSocket->write(compressedData);
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
				compressedData=localSocket.at(indexOfSelection())->compression->compressData(block);
			byteWriten=localSocket.at(indexOfSelection())->localSocket->write(compressedData);
			if(byteWriten<=0)
				ErrorQString=localSocket.at(indexOfSelection())->localSocket->errorString();
		}
	}
	if(byteWriten!=compressedData.size()) //check if all the byte have been writen
	{
		if(byteWriten==-1)
			ui->statusBarApp->showMessage(tr("Byte to write: ")+QString::number(compressedData.size())+"\n"+tr("Byte realy writen: ")+QString::number(byteWriten)+"\n"+tr("Error message: ")+ErrorQString);
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
	return false;
}

void MainWindow::on_pushButtonSend_clicked()
{
	if(indexOfSelection()!=-1)
		sendNewData();
}

void MainWindow::on_compressionType_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	switch(ui->compressionType->currentIndex())
	{
		case 2:
		case 3:
			ui->compressionOptions->setHidden(false);
			ui->compressionBufferSize->setHidden(false);
			ui->label_compressio_buffer->setHidden(false);
			ui->compressionByPacket->setHidden(false);
			ui->label_compression_by_packet->setHidden(false);
		break;
		case 0:
		case 1:
		default:
			ui->compressionOptions->setHidden(true);
			ui->compressionBufferSize->setHidden(true);
			ui->label_compressio_buffer->setHidden(true);
			ui->compressionByPacket->setHidden(true);
			ui->label_compression_by_packet->setHidden(true);
		break;
	}
}
