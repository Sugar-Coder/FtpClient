#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTreeWidgetItem>

#include "myftp.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    connect(ui->fileList, &QTreeWidget::itemActivated,
            this, &MainWindow::processItem);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    case MyFtp::ConnectToHost :
        ui->label->setText(tr("正在连接到服务器…"));
        break;
    case MyFtp::Login :
        ui->label->setText(tr("正在登录…"));
        break;
    case MyFtp::Get :
        ui->label->setText(tr("正在下载…"));
        break;
    case MyFtp::Close :
        ui->label->setText(tr("正在关闭连接…"));
    }
}

/**
 * @brief MainWindow::ftpCommandFinished
 * @param error
 * 根据命令执行结果显示信息
 */
void MainWindow::ftpCommandFinished(int, bool error){
    if(ftp->currentCommand() == MyFtp::ConnectToHost){
        if (error)
            ui->ftpLabel->setText(
                        tr("连接服务器出现错误：%1").arg(ftp->errorString()));
        else {
            ui->connectButton->setDisabled(true);
            ui->connectButton->setText("已连接");
            ui->ftpLabel->setText(tr("连接服务器成功"));
        }
    } else if (ftp->currentCommand() == MyFtp::Login) {
        if (error){
            ui->ftpLabel->setText(tr("登录出现错误：%1").arg(ftp->errorString()));
        }
        else {
            ui->downloadButton->setEnabled(true);
            ui->uploadButton->setEnabled(true);
            ui->ftpLabel->setText(tr("登录成功"));
            ftp->list(); // 显示目录结构
        }
    } else if (ftp->currentCommand() == MyFtp::Get){
        if(error)
            ui->ftpLabel->setText(tr("下载出现错误：%1")
                                  .arg(ftp->errorString()));
        else{
            ui->ftpLabel->setText(tr("已经完成下载"));
            file->close();
        }
        ui->downloadButton->setEnabled(true);
    } else if (ftp->currentCommand() == MyFtp::List){
        if (isDirectory.isEmpty()){
            ui->fileList->addTopLevelItem(
                        new QTreeWidgetItem(QStringList() << tr("<empty>")));
            ui->fileList->setEnabled(false);
            ui->ftpLabel->setText(tr("该目录为空"));
        }
    } else if(ftp->currentCommand() == MyFtp::Close){
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
    ui->progressBar->setValue(0);
    ftp = new MyFtp();
    connect(ftp, &MyFtp::commandStarted, this,
            &MainWindow::ftpCommandStarted); // 显示当前ftp执行的命令
    connect(ftp, &MyFtp::commandFinished, this,
            &MainWindow::ftpCommandFinished); // 显示命令执行结果
    connect(ftp, &MyFtp::listInfo, this, &MainWindow::addToList); // 显示文件列表
    connect(ftp, &MyFtp::dataTransferProgress,
            this, &MainWindow::updateDataTransferProgress); // 更新传输进度条
    QString ftpServer = ui->ftpServerlineEdit->text();
    QString userName = ui->userNamelineEdit->text();
    QString password = ui->passwordlineEdit->text();
    ftp->connectToHost(ftpServer, 21);
    ftp->login(userName, password);
}

/**
 * @brief MainWindow::addToList
 * 将ftp返回的目录信息加入到QTreeWidgetItem中，并且对于目录保存到isDirectory中
 * @todo: 确定ftp信号传入的参数类型
 */
void MainWindow::addToList(){

}

/**
 * @brief MainWindow::processItem
 * @param item：被双击的目录项
 * cd到item对应的目录
 */
void MainWindow::processItem(QTreeWidgetItem * item, int){
    if (isDirectory.value(item->text(0))) {
        QString name = QLatin1String(item->text(0).toUtf8());
        ui->fileList->clear();
        isDirectory.clear();
        currentPath += "/";
        currentPath += name;
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
    ui->fileList->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    if (currentPath.isEmpty()) {
        ui->cdToParentButton->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(currentPath);
    }
    ftp->list();
}

/**
 * @brief MainWindow::on_downloadButton_clicked
 * 对于被选中的item，点按下载按钮后进行下载
 */
void MainWindow::on_downloadButton_clicked()
{
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
