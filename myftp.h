#ifndef MYFTP_H
#define MYFTP_H

class QFile;

class MyFtp
{
private:
    bool error = false; // 当前执行是否出错
    int cur_command = -1; // 当前执行的指令
public:
    void cd(QString directory); // 变更当前目录
    void list(); // 获取当前目录（并发送信号listInfo）
    void get(QString filename, QFile * file); // 下载文件
public:
    MyFtp();
    enum{ConnectToHost, Login, Get, List, Close}; // 指令
    connectToHost(QString server, int port);
    login(QString username, QString password);
    QString errorString(); // 获取错误信息
    int currentCommand(); // 获取当前指令
signals:
    void commandStarted(int);
    void commandFinished(int, bool);
    void listInfo(); // TODO:参数类型,返回当前目录信息
    void dataTransferProgress(); // TODO:参数类型，文件下载进度
};

#endif // MYFTP_H
