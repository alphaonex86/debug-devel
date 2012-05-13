#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <time.h>
#include <sys/timeb.h>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	compressor=NULL;
	decompressor=NULL;
}

MainWindow::~MainWindow()
{
	if(compressor!=NULL)
		delete compressor;
	if(decompressor!=NULL)
		delete decompressor;
	delete ui;
}

void MainWindow::on_add_clicked()
{
	bool ok;
	QString name=QInputDialog::getText(this,tr("Give the name"),tr("Name"),QLineEdit::Normal,QString(),&ok);
	if(!ok)
		return;
	if(name.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),"The name can't be empty!");
		return;
	}

	QString hexa=QInputDialog::getText(this,tr("Give the hexa"),tr("Hexa"),QLineEdit::Normal,QString(),&ok);
	if(!ok)
		return;
	if(hexa.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),"The hexa code can't be empty!");
		return;
	}
	QByteArray hexaData=QByteArray::fromHex(hexa.toAscii());
	if(hexaData.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),tr("The hexa seam wrong: %1").arg(hexa));
		return;
	}

	int count=QInputDialog::getInt (this,tr("Give the count"),tr("Occurence"),1,1,2147483647,1,&ok);
	if(!ok)
		return;

	data newItem;
	newItem.count=count;
	newItem.data=hexaData;
	newItem.name=name;
	data_list << newItem;
	updateShowedItems();
}

void MainWindow::updateShowedItems()
{
	ui->treeWidget->clear();
	int index=0;
	while(index<data_list.size())
	{
		ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << data_list.at(index).name << QString::number(data_list.at(index).count)));
		index++;
	}
}

void MainWindow::on_remove_clicked()
{
	if(ui->treeWidget->selectedItems().size()!=1)
		return;
	int index=ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->selectedItems().last());
	if(index==-1)
		return;
	data_list.removeAt(index);
	delete ui->treeWidget->selectedItems().last();
}

void MainWindow::on_load_clicked()
{
	//save file here
	QString file = QFileDialog::getOpenFileName(
		this,
		tr("Select one file to open"),
		"~/",
		tr("All files")+" (*)");
	if(file=="")
	{
		ui->statusBar->showMessage("Canceled!",10000);
		return;
	}
	QSettings settings(file,QSettings::IniFormat);
	bool ok;
	int index=0;
	while(true)
	{
		if(!settings.contains(QString("item%1_count").arg(index)))
			break;
		if(!settings.contains(QString("item%1_data").arg(index)))
			break;
		if(!settings.contains(QString("item%1_name").arg(index)))
			break;
		data newItem;
		newItem.count=settings.value(QString("item%1_count").arg(index),1).toUInt(&ok);
		if(!ok)
			break;
		newItem.data=settings.value(QString("item%1_data").arg(index),1).toByteArray();
		if(!ok)
			break;
		newItem.name=settings.value(QString("item%1_name").arg(index),1).toString();
		if(!ok)
			break;
		data_list << newItem;
		index++;
	}
	updateShowedItems();
}

void MainWindow::on_save_clicked()
{
	//load file here
	QString file = QFileDialog::getOpenFileName(
		this,
		tr("Select one file to open"),
		"~/",
		tr("All files")+" (*)");
	if(file=="")
	{
		ui->statusBar->showMessage("Canceled!",10000);
		return;
	}
	QSettings settings(file,QSettings::IniFormat);
	settings.clear();
	int index=0;
	while(index<data_list.size())
	{
		settings.setValue(QString("item%1_name").arg(index),data_list.at(index).name);
		settings.setValue(QString("item%1_data").arg(index),data_list.at(index).data);
		settings.setValue(QString("item%1_count").arg(index),data_list.at(index).count);
		index++;
	}
}

void MainWindow::on_benchmark_clicked()
{
	int compressionTime=0,decompressionTime=0;
	quint64 transferedSize=0,rawSize=0;
	QTime time;
	if(compressor!=NULL)
		delete compressor;
	if(decompressor!=NULL)
		delete decompressor;
	compressor=NULL;
	decompressor=NULL;
	int compressionLevel=1;
	switch(ui->compressionType->currentIndex())
	{
		case 4:
			compressor=new GzipCompressionTcpSocket(65536,compressionLevel);
			decompressor=new GzipCompressionTcpSocket(65536,compressionLevel);
		break;
		case 3:
			compressor=new ZlibCompressionTcpSocket(65536,compressionLevel);
			decompressor=new ZlibCompressionTcpSocket(65536,compressionLevel);
		break;
		case 2:
			compressor=new Lz4CompressionTcpSocket();
			decompressor=new Lz4CompressionTcpSocket();
		break;
		case 1:
			compressor=new Lz4HcCompressionTcpSocket();
			decompressor=new Lz4HcCompressionTcpSocket();
		break;
		case 0:
			compressor=new NoCompressionTcpSocket();
			decompressor=new NoCompressionTcpSocket();
		default:
		break;
	}
	QByteArray compressedData,decompressedData,chunk;
	timespec tstart,tstop;
	int count;
	int index=0;
	while(index<data_list.size())
	{
		count=0;
		while(count<data_list.at(index).count)
		{
			qDebug() << QString("data_list.at(index).data.size(): %1").arg(data_list.at(index).data.size());
			rawSize+=data_list.at(index).data.size();
			clock_gettime(CLOCK_REALTIME, &tstart);
			compressedData=compressor->compressData(data_list.at(index).data);
			clock_gettime(CLOCK_REALTIME, &tstop);
			compressionTime+=((quint64)tstop.tv_sec * 1000000LL + (quint64)tstop.tv_nsec / 1000LL)
					-((quint64)tstart.tv_sec * 1000000LL + (quint64)tstart.tv_nsec / 1000LL);
			transferedSize+=compressedData.size();
			qDebug() << QString("compressedData.size(): %1").arg(compressedData.size());
			qDebug() << QString("compressedData: %1").arg(QString(compressedData.toHex()));
			if(ui->cutCompressedpacket->isChecked())
			{
				decompressedData.clear();
				while(compressedData.size()>0)
				{
					if(compressedData.size()>1500)
						chunk=compressedData.mid(0,1500);
					else
						chunk=compressedData;
					compressedData.remove(0,chunk.size());
					clock_gettime(CLOCK_REALTIME, &tstart);
					decompressedData+=decompressor->decompressData(chunk);
					clock_gettime(CLOCK_REALTIME, &tstop);
					qDebug() << QString("decompressData()");
					decompressionTime+=((quint64)tstop.tv_sec * 1000000LL + (quint64)tstop.tv_nsec / 1000LL)
							-((quint64)tstart.tv_sec * 1000000LL + (quint64)tstart.tv_nsec / 1000LL);
				}
			}
			else
			{
				clock_gettime(CLOCK_REALTIME, &tstart);
				decompressedData=decompressor->decompressData(compressedData);
				clock_gettime(CLOCK_REALTIME, &tstop);
				decompressionTime+=((quint64)tstop.tv_sec * 1000000LL + (quint64)tstop.tv_nsec / 1000LL)
						-((quint64)tstart.tv_sec * 1000000LL + (quint64)tstart.tv_nsec / 1000LL);
			}
			qDebug() << QString("decompressedData.size(): %1").arg(decompressedData.size());
			if(decompressedData!=data_list.at(index).data)
			{
				ui->statusBar->showMessage("The data not match!",10000);
				return;
			}
			count++;
		}
		index++;
	}
	if(data_list.size()>0)
	{
		QString ratioCompressionSpeed="Na";
		if(decompressionTime>0)
			ratioCompressionSpeed=QString::number((double)compressionTime/(double)decompressionTime);
		QString ratioCompression="Na";
		if(rawSize>0)
			ratioCompression=QString::number((double)transferedSize/(double)rawSize);
		QMessageBox::information(this,tr("Benchmark"),
		QString("compressionTime: %1, decompressionTime: %2\ncompression speed/decompression speed ratio: %3\n\nrawSize: %4, transferedSize: %5\ncompression ratio: %6")
			.arg(compressionTime)
			.arg(decompressionTime)
			.arg(ratioCompressionSpeed)
			.arg(rawSize)
			.arg(transferedSize)
			.arg(ratioCompression)
			);
	}
	else
		QMessageBox::critical(this,tr("Error"),"No data for benchmark");
}

void MainWindow::on_edit_clicked()
{
	if(ui->treeWidget->selectedItems().size()!=1)
		return;
	int index=ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->selectedItems().last());
	if(index==-1)
		return;

	bool ok;
	QString name=QInputDialog::getText(this,tr("Give the name"),tr("Name"),QLineEdit::Normal,data_list.at(index).name,&ok);
	if(!ok)
		return;
	if(name.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),"The name can't be empty!");
		return;
	}

	QString hexa=QInputDialog::getText(this,tr("Give the hexa"),tr("Hexa"),QLineEdit::Normal,data_list.at(index).data.toHex(),&ok);
	if(!ok)
		return;
	if(hexa.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),"The hexa code can't be empty!");
		return;
	}
	QByteArray hexaData=QByteArray::fromHex(hexa.toAscii());
	if(hexaData.isEmpty())
	{
		QMessageBox::critical(this,tr("Error"),tr("The hexa seam wrong: %1").arg(hexa));
		return;
	}

	int count=QInputDialog::getInt (this,tr("Give the count"),tr("Occurence"),data_list.at(index).count,1,2147483647,1,&ok);
	if(!ok)
		return;

	data_list[index].count=count;
	data_list[index].data=hexaData;
	data_list[index].name=name;
	updateShowedItems();
}
