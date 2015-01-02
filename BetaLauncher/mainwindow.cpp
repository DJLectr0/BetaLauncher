#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    QUrl imageUrl("http://leonardogalli.ch/beta/list.json");
    fileDownloader = new FileDownloader(imageUrl, this);

    connect(fileDownloader, SIGNAL(downloaded()), SLOT(fileDownloaded()));
    ui->setupUi(this);
    ui->comboBox->addItem("Loading Items");
    ui->listWidget->addItem("15 of December 2014:\nRelease of Beta Launcher");
    ui->progressBar->setHidden(true);
    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(handleButton()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectedChange(QString)));


}
void MainWindow::fileDownloaded()
{
    QByteArray m_DownloadedData = fileDownloader->downloadedData();
    printf("data: %s\n",fileDownloader->downloadedData().data());
    QJsonDocument loadDoc = QJsonDocument::fromJson(m_DownloadedData);
    //printf(loadDoc.object()["test2"].toObject()["version"].toString().toLocal8Bit().data());
    QStringList Items = loadDoc.object().keys();
    //Items.removeAt(0);
    ui->comboBox->removeItem(0);
    ui->comboBox->addItems(Items);
    //printf("\n %s", Items.at(0));
    QFile loadFile("list.json");


        printf("File does not exist");
        loadFile.open(QIODevice::ReadWrite);



     printf("File does not exist");
    //QFile loadFile("list.json");
    QByteArray list = loadFile.readAll();
    QJsonDocument loadDoc2(QJsonDocument::fromJson(list));
    QJsonObject root;
    foreach(QString item, Items)
    {
        QJsonObject itemJ;
        if(loadDoc2.object().keys().contains(item))
        {
            //All ready parsed
            //printf("\nparsed: %s", loadDoc.object()[item].toObject()["time"]);
            //printf("\nloaded: %s", itemJ["time"]);

            itemJ = loadDoc2.object()[item].toObject();
            if(itemJ["time"] != loadDoc.object()[item].toObject()["time"])
            {
                //itemJ["bNeedsUpdate"] = "true";
                this->needsUpdateList.append(item);
                this->times.append(loadDoc.object()[item].toObject()["time"].toString());
            }
            foreach(QString key, loadDoc.object()[item].toObject().keys())
            {
                if(!(key == "time" && this->needsUpdateList.contains(item)))
                {
                    itemJ[key] = loadDoc.object()[item].toObject()[key];
                }
            }
        }else{
            foreach(QString key, loadDoc.object()[item].toObject().keys())
            {
                itemJ[key] = loadDoc.object()[item].toObject()[key];
                //printf("\nkey: %s", key);
            }
            //itemJ["installed"] = "false";
        }
        root[item] = itemJ;
    }
    QJsonDocument writeDoc(root);
    loadFile.resize(0);
    loadFile.write(writeDoc.toJson());
    loadFile.close();
    this->list = root;
    QJsonObject item = this->list[ui->comboBox->currentText()].toObject();
    if(!this->isInstalled((ui->comboBox->currentText())))
    {
        ui->pushButton->setText("Install");
        ui->label_3->setText(this->list[ui->comboBox->currentText()].toObject()["description"].toString());

    }
    else if(this->needsUpdateList.contains(ui->comboBox->currentText()))
    {
        ui->pushButton->setText("Update");
        ui->label_3->setText("Changelog:\n" + this->list[ui->comboBox->currentText()].toObject()["changelog"].toString());
    }
}
void MainWindow::handleButton()
{
    QJsonObject item = this->list[ui->comboBox->currentText()].toObject();
    if(this->isInstalled(ui->comboBox->currentText())&&!this->needsUpdateList.contains(ui->comboBox->currentText()))
    {
        //Find exe and launch, also ensure that steam is running or else launch it
        const QStringList nameFilter(ui->comboBox->currentText() + ".exe");
        QDir dir("Apps/" + ui->comboBox->currentText());
        QDirIterator dirIt("Apps/" + ui->comboBox->currentText(),nameFilter, QDir::NoFilter, QDirIterator::Subdirectories);
        while (dirIt.hasNext()) {
            dirIt.next();

                    qDebug()<<dirIt.filePath();
                    QProcess *myProcess = new QProcess(this);
                    QStringList arguments;
                    myProcess->start(dirIt.fileInfo().absoluteFilePath(), arguments);

                    ui->centralWidget->hide();
                    myProcess->waitForFinished(-1);
                    ui->centralWidget->show();
                    qDebug() <<  myProcess->errorString();
                    myProcess->dumpObjectInfo();
        }
    }else{
        if(this->needsUpdateList.contains(ui->comboBox->currentText()))
        {
            if (QMessageBox::No == QMessageBox(QMessageBox::Information, "Update available", "Do you want to update " + ui->comboBox->currentText() + "?", QMessageBox::Yes|QMessageBox::No).exec())
            {
                //Find exe and launch, also ensure that steam is running or else launch it
                const QStringList nameFilter(ui->comboBox->currentText() + ".exe");
                QDir dir("Apps/" + ui->comboBox->currentText());
                QDirIterator dirIt("Apps/" + ui->comboBox->currentText(),nameFilter, QDir::NoFilter, QDirIterator::Subdirectories);
                while (dirIt.hasNext()) {
                    dirIt.next();

                            qDebug()<<dirIt.filePath();
                            QProcess *myProcess = new QProcess(this);
                            QStringList arguments;
                            myProcess->start(dirIt.fileInfo().absoluteFilePath(), arguments);

                            ui->centralWidget->hide();
                            myProcess->waitForFinished(-1);
                            ui->centralWidget->show();
                            qDebug() <<  myProcess->errorString();
                            myProcess->dumpObjectInfo();
                }
                return;
            }
        }
        install(this->needsUpdateList.contains(ui->comboBox->currentText()), ui->comboBox->currentText());
    }
}
void MainWindow::install(bool updating, QString item)
{

    ui->progressBar->setHidden(false);
    QString rootPath;
    rootPath = "Apps";
    if(!updating)
    {

#ifdef Q_WS_MACX
        rootPath = QDesktopServices::storageLocation( QDesktopServices::DataLocation )+"BetaLauncher";
#endif
        QDir path = QDir("");
       path.mkpath(rootPath + "/" + item);

    }
    this->hashDownloader = new FileDownloader(QUrl("http://leonardogalli.ch/beta/builds/hash" + item + ".json"));
    connect(this->hashDownloader, SIGNAL(downloaded()), SLOT(hashDownloaded()));
    ui->pushButton->setEnabled(false);
    ui->label->setText("Inizializing");
    ui->label_3->setText("");
    printf("Installing");

}
void MainWindow::downloadProgCalc(QString remainingTime, QString Speed, int percentage)
{
    ui->progressBar->setValue(percentage);
    ui->label->setText("Downloading with "+ Speed + ", " + remainingTime + " remaining");
}
void MainWindow::selectedChange(QString item)
{
    QJsonObject itemJ = this->list[item].toObject();
    if(!this->isInstalled((item)))
    {
        ui->pushButton->setText("Install");
        ui->label_3->setText("Changelog: " + this->list[ui->comboBox->currentText()].toObject()["description"].toString());

    }
    else if(this->needsUpdateList.contains(item))
    {
        ui->pushButton->setText("Update");
        ui->label_3->setText("Changelog: " + this->list[ui->comboBox->currentText()].toObject()["changelog"].toString());

    }
    else
    {
        ui->pushButton->setText("Launch");
        ui->label_3->setText("");
    }

}
void MainWindow::hashDownloaded()
{
    QByteArray m_DownloadedData = hashDownloader->downloadedData();
    this->dlListData = hashDownloader->downloadedData();
    printf("data: %s\n",hashDownloader->downloadedData().data());
    QJsonDocument loadDoc = QJsonDocument::fromJson(m_DownloadedData);
    QJsonObject hashes = loadDoc.object();
    QStringList toDownload;
    QString item = ui->comboBox->currentText();
    QDirIterator it("Apps/"+item, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString fileL = it.next();

        QString relativeFile = fileL.replace("Apps/", "");
        QString hash = this->hash("Apps/" + relativeFile);
        if(!hashes.contains(relativeFile))
        {
            qDebug() << "Remove file: " << relativeFile;
            QFile oldFile("Apps/" + relativeFile);
            oldFile.remove();
        }
        else
        {
        qDebug() << "Hash of file: " << fileL << ", " << hash << " online says: " << hashes[relativeFile].toString();
        if(hash == hashes[relativeFile].toString())
        {
            hashes.remove(relativeFile);
        }

        }
    }
    QString rootPath = "Apps";
    DownloadManager* manager = new DownloadManager(this);
    int count = 0;
    foreach(QString file, hashes.keys())
    {
        count++;
        qDebug() << "Adding file: %s" << "http://leonardogalli.ch/beta/builds/" << file;
        manager->addItem(QUrl("http://leonardogalli.ch/beta/builds/"+ file));
        manager->addPath(rootPath + "/" + file);

    }


    if(count==0)
    {
        this->downloadManagerFinished();
    }
    connect(manager, SIGNAL(progress(QString,QString,int)), SLOT(downloadProgCalc(QString,QString,int)));
    connect(manager, SIGNAL(finished()), SLOT(downloadManagerFinished()));
    manager->start();
}
QString MainWindow::hash(QString file)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile in(file);
    qDebug() << "Hashing file: " << file;
    if (in.open(QIODevice::ReadOnly)) {
        char buf[2048];
        int bytesRead;
        qint64 overallBytesRead = 0;
        if(in.size()<2048)
        {
            hash.addData(in.readAll());
        }
        else
        {
        while ((bytesRead = in.read(buf, 2048)) > 0) {
            overallBytesRead += bytesRead;
            hash.addData(buf, bytesRead);
        }
        }
        in.close();

    }
    else
    {

    }
    return hash.result().toHex();
}
void MainWindow::downloadManagerFinished()
{

    ui->progressBar->setHidden(true);
    ui->label->setText("");
    ui->progressBar->setValue(0);
    this->list[ui->comboBox->currentText()].toObject().remove("installed");
    //this->list[ui->comboBox->currentText()].toObject()["installed"] = "true";

    QString installed = this->list[ui->comboBox->currentText()].toObject()["installed"].toString();

    this->list[ui->comboBox->currentText()].toObject().remove("bNeedsUpdate");
    //this->list[ui->comboBox->currentText()].toObject()["bNeedsUpdate"] = "false";
    //this->selectedChange(ui->comboBox->currentText());
    QFile loadFile("list.json");

    if(loadFile.open(QIODevice::ReadWrite))
    {

    }else{
        qDebug() << "Failed to open list";
    }


    ui->pushButton->setEnabled(true);
    this->list[ui->comboBox->currentText()].toObject().remove("time");
    this->list[ui->comboBox->currentText()].toObject()["time"] = this->times.at(this->needsUpdateList.indexOf(ui->comboBox->currentText()));
    this->needsUpdateList.removeAt(this->needsUpdateList.indexOf(ui->comboBox->currentText()));
    this->selectedChange(ui->comboBox->currentText());
    QJsonDocument writeDoc(this->list);
    loadFile.resize(0);
    loadFile.write(writeDoc.toJson());
    loadFile.close();

}
bool MainWindow::isInstalled(QString item)
{
        return QDir("Apps/" + item).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}
