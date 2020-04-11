#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QHash>

class QFtp;
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
    Ui::MainWindow *ui;
    QFtp * ftp;
    // 用于存储一个路径是否为目录的信息
    QHash <QString, bool> isDirectory;
    // 存储当前路径
    QString currentPath;
    // 用来表示要下载的文件
    QFile * file;
    // 本地上传的文件
    QFile * localFile;
    QString filename;
    // treeview的右键菜单
    QMenu *m_server_menu;

    int indexCount;
    int currentIndex;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // 初始化界面显示
    void initDisplay();

private slots:
    void ftpCommandStarted(int); // 接受ftp的命令
    void ftpCommandFinished(int, bool); // ftp的命令执行结束，bool变量表示执行成功与否

    // 更新进度条
    void updateDataTransferProgress(qint64, qint64);
    // 将服务器上的文件添加到TreeWidget部件中 TODO:参数类型
    void addToList(const QUrlInfo &urlInfo);
    // 双击一个目录时显示其内容
    void processItem(QTreeWidgetItem*, int);

    // 按钮
    void on_connectButton_clicked();
    void on_cdToParentButton_clicked();
    void on_downloadButton_clicked();

    // TreeView显示右键菜单
    void showFtpTreeViewMenu(const QPoint &point);
    // 右键的功能
    void slotMkdir();
    void slotDeleteFile();
    void slotRefreshFtpList();

    void openFile();
    void uploadLocalFile();
    void on_openButton_clicked();
    void on_uploadButton_clicked();
};

#endif // MAINWINDOW_H
