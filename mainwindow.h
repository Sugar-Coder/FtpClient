#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MyFtp; // 自己实现的ftp

#include <QHash>
class QFile;
class QUrlInfo;
class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    MyFtp * ftp; // ftp对象
    // 用于存储一个路径是否为目录的信息
    QHash <QString, bool> isDirectory;
    // 存储当前路径
    QString currentPath;
    // 用来表示要下载的文件
    QFile * file;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void ftpCommandStarted(int); // param: ftp的命令
    void ftpCommandFinished(int, bool); // param: ftp的命令和返回成功

    // 更新进度条
    void updateDataTransferProgress(qint64, qint64);
    // 将服务器上的文件添加到TreeWidget部件中
    void addToList(const QUrlInfo &urlInfo);
    // 双击一个目录时显示其内容
    void processItem(QTreeWidgetItem*, int);
    void on_connectButton_clicked();
};

#endif // MAINWINDOW_H
