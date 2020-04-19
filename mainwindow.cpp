#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QFileDialog>

#include "qftp.h"
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initDisplay();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDisplay(){
    ui->cdToParentButton->setEnabled(false);
    ui->downloadButton->setEnabled(false);
    ui->uploadButton->setEnabled(false);
    ui->progressBar->setValue(0);

    ui->fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    this->m_server_menu = new QMenu(this);

    QAction * server_rm = new QAction(QObject::tr("删除"), this);
    this->m_server_menu->addAction(server_rm);
}
/**
 * @brief MainWindow::ftpCommandStarted
 * 根据ftp执行的命令显示提示
 */
void MainWindow::ftpCommandStarted(int)
{
    int id = ftp->currentCommand();
    switch (id)
    {
    case QFtp::ConnectToHost :
        ui->ftpLabel->setText(tr("正在连接到服务器…"));
        break;
    case QFtp::Login :
        ui->ftpLabel->setText(tr("正在登录…"));
        break;
    case QFtp::Get :
        ui->ftpLabel->setText(tr("正在下载…"));
        break;
    case QFtp::Close :
        ui->ftpLabel->setText(tr("正在关闭连接…"));
    }
}

/**
 * @brief MainWindow::ftpCommandFinished
 * @param error
 * 根据命令执行结果显示信息
 */
void MainWindow::ftpCommandFinished(int, bool error){
    if(ftp->currentCommand() == QFtp::ConnectToHost){
        if (error)
            ui->ftpLabel->setText(
                        tr("连接服务器出现错误：%1").arg(ftp->errorString()));
        else {
            ui->connectButton->setDisabled(true);
            ui->connectButton->setText("已连接");
            ui->ftpLabel->setText(tr("连接服务器成功"));
        }
    } else if (ftp->currentCommand() == QFtp::Login) {
        if (error){
            ui->ftpLabel->setText(tr("登录出现错误：%1").arg(ftp->errorString()));
        }
        else {
            ui->downloadButton->setEnabled(true);
            ui->uploadButton->setEnabled(true);
            ui->ftpLabel->setText(tr("登录成功"));
            ftp->list(); // 显示目录结构
        }
    } else if (ftp->currentCommand() == QFtp::Get){
        if(error)
            ui->ftpLabel->setText(tr("下载出现错误：%1")
                                  .arg(ftp->errorString()));
        else{
            ui->ftpLabel->setText(tr("已经完成下载"));
            file->close();
        }
        ui->downloadButton->setEnabled(true);
    } else if (ftp->currentCommand() == QFtp::List){
        if (isDirectory.isEmpty()){
            ui->fileList->addTopLevelItem(
                        new QTreeWidgetItem(QStringList() << tr("<empty>")));
            ui->fileList->setEnabled(false);
            ui->ftpLabel->setText(tr("该目录为空"));
        }
    } else if(ftp->currentCommand() == QFtp::Put) {
        if(error)
            ui->label->setText(tr("上传出现错误：检查文件是否重名！").arg(ftp->errorString()));
        else {
            ui->label->setText(tr("上传完成"));
            file->close();
            // 只考虑单个文件的上传下载
            isDirectory.clear();
            ui->fileList->clear();
            ftp->list();
        }
    } else if (ftp->currentCommand() == QFtp::Mkdir){
        ui->label->setText(tr("新建文件夹完成"));
        //刷新显示列表
        isDirectory.clear();
        ui->fileList->clear();
        ftp->list();
    }else if (ftp->currentCommand() == QFtp::Remove){
        // currentIndex++;
        // if(currentIndex >= indexCount){
            ui->label->setText(tr("删除完成！"));
            isDirectory.clear();
            ui->fileList->clear();
            ftp->list();
        // }
    }else if(ftp->currentCommand() == QFtp::Rmdir){
        currentIndex++;
        if(currentIndex >= indexCount){
            ui->label->setText(tr("删除完成！"));
            isDirectory.clear();
            ui->fileList->clear();
            ftp->list();
        }
    }else if(ftp->currentCommand() == QFtp::Close){
        ui->ftpLabel->setText(tr("已经关闭连接"));
    }
}

/**
 * @brief MainWindow::on_connectButton_clicked
 * 用于关联信号和槽，以及登陆服务器
 */
void MainWindow::on_connectButton_clicked()
{
    ui->fileList->clear();
    currentPath.clear();
    isDirectory.clear();

    ftp = new QFtp(this);
    connect(ftp, &QFtp::commandStarted, this,
            &MainWindow::ftpCommandStarted); // 显示当前ftp执行的命令
    connect(ftp, &QFtp::commandFinished, this,
            &MainWindow::ftpCommandFinished); // 显示命令执行结果
    connect(ftp, &QFtp::listInfo, this, &MainWindow::addToList); // 显示文件列表
    connect(ftp, &QFtp::dataTransferProgress,
            this, &MainWindow::updateDataTransferProgress); // 更新传输进度条
    connect(ui->fileList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showFtpTreeViewMenu(QPoint)));
    // 双击进入目录
    connect(ui->fileList, &QTreeWidget::itemActivated,
            this, &MainWindow::processItem);

    QString ftpServer = ui->ftpServerlineEdit->text();
    QString userName = ui->userNamelineEdit->text();
    QString password = ui->passwordlineEdit->text();
    ftp->connectToHost(ftpServer, 21);
    ftp->login(userName, password);
}

/**
 * @brief MainWindow::addToList 这是一个槽，等待着ftp发送信息
 * 将ftp返回的目录信息加入到QTreeWidgetItem中，并且对于目录保存到isDirectory中
 * @todo: 确定ftp信号传入的参数类型
 */
void MainWindow::addToList(const QUrlInfo & urlInfo){
    qDebug()<< "显示文件列表";
    QString name = QString::fromUtf8(urlInfo.name().toLatin1());
    QString owner = QString::fromUtf8(urlInfo.owner().toLatin1());
    QString group = QString::fromUtf8(urlInfo.group().toLatin1());
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, owner);
    item->setText(3, group);
    item->setText(4, urlInfo.lastModified().toString("yyy-MM-dd"));
    QPixmap pixmap(urlInfo.isDir() ? ":/dir.png" : ":/file.png");
    item->setIcon(0, pixmap);
    isDirectory[name] = urlInfo.isDir();
    ui->fileList->addTopLevelItem(item);
    if(!ui->fileList->currentItem()){
        ui->fileList->setCurrentItem(ui->fileList->topLevelItem(0));
        ui->fileList->setEnabled(true);
    }
}

/**
 * @brief MainWindow::processItem
 * @param item：被双击的目录项
 * cd到item对应的目录
 */
void MainWindow::processItem(QTreeWidgetItem * item, int){
    if(item->isDisabled())
        return;
    if (isDirectory.value(item->text(0))) {
        QString name = QLatin1String(item->text(0).toUtf8());
        ui->fileList->clear();
        isDirectory.clear();
        currentPath += "/";
        currentPath += name;
        qDebug() << "currentPath" << currentPath;
        ftp->cd(name);
        ftp->list(); // 显示新的目录结构
        ui->cdToParentButton->setEnabled(true);
    }
}

/**
 * @brief MainWindow::on_cdToParentButton_clicked
 * 返回上级目录
 */
void MainWindow::on_cdToParentButton_clicked()
{
    qDebug() << "re:currentPath:" << currentPath;
    ui->fileList->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    qDebug()<<"now:currentPath:"<<currentPath;
    if (currentPath.isEmpty()) {
        ui->cdToParentButton->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(currentPath);
    }
    ftp->list(); // 更新目录
}

/**
 * @brief MainWindow::on_downloadButton_clicked
 * 对于被选中的item，点按下载按钮后进行下载
 */
void MainWindow::on_downloadButton_clicked()
{
    if(isDirectory.isEmpty())
        return;
    QString fileName = ui->fileList->currentItem()->text(0);
    QString name = QLatin1String(fileName.toUtf8());
    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        delete file;
        return;
    }
    ui->downloadButton->setEnabled(false);
    ftp->get(name, file);
}

void MainWindow::updateDataTransferProgress(qint64 readBytes, qint64 totalBytes){
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(readBytes);
}

// 右键filelist
void MainWindow::showFtpTreeViewMenu(const QPoint &point){
    QMenu* menu = new QMenu;
    menu->addAction(QString(tr("新建文件夹")), this, SLOT(slotMkdir()));
    menu->addAction(QString(tr("刷新")),this, SLOT(slotRefreshFtpList()));
    menu->addAction(QString(tr("删除")), this, SLOT(slotDeleteFile()));
    menu->exec(QCursor::pos());
}
// 以下是右键菜单功能
// 刷新filelist
void MainWindow::slotRefreshFtpList(){
    isDirectory.clear();
    ui->fileList->clear();
    ftp->list();
}

// 新建目录
void MainWindow::slotMkdir(){
    qDebug() << "新建文件夹中...";
    QString dirName = QInputDialog::getText(this, tr("新建文件夹"),
                                            tr("文件夹名称"));
    if(dirName.isEmpty() == false){
        ftp->mkdir(QLatin1String(dirName.toUtf8()));
    }
}

void MainWindow::slotDeleteFile(){
    QModelIndexList indexList = ui->fileList->selectionModel()->selectedRows();
    QString fileName;
    currentIndex = 0;
    indexCount = indexList.count();
    ui->label->setText("正在删除....");
    for(int i = 0;i < indexCount;i++)
    {
        fileName = indexList.at(i).data().toString();
        if(isDirectory.value(fileName))
            ftp->rmdir(QLatin1String(fileName.toUtf8()));
        else
            ftp->remove(QLatin1String(fileName.toUtf8()));
    }
}

// 打开本地文件
void MainWindow::openFile(){
    filename = QFileDialog::getOpenFileName(this);
    if(!filename.isEmpty()) {
        ui->uploadButton->setEnabled(true);
        ui->localFileLabel->setText(tr("打开文件 %1 成功").arg(filename));
    }
}

void MainWindow::on_openButton_clicked()
{
    ui->localFileLabel->setText(tr("等待打开文件！"));
    openFile();
}

// 文件上传
void MainWindow::uploadLocalFile(){
    localFile = new QFile(filename);
    if(!file->open(QIODevice::ReadOnly)) {
        delete localFile;
        return;
    }
    QString name = QLatin1String(filename.toUtf8());
    ftp->put(localFile, name);
    filename = "";
}

void MainWindow::on_uploadButton_clicked()
{
    if(filename.isEmpty())
        return;
    uploadLocalFile();
}
