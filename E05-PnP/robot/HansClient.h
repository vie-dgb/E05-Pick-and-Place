#ifndef HANSCLIENT_H
#define HANSCLIENT_H

#include <chrono>
#include <QThread>
#include <QObject>
#include <QTcpSocket>
#include <QMutex>

#include "HansCommand.h"

using namespace std::chrono;

namespace rb {

#define HANS_COMMAND_PORT           10003
#define HANS_CONNECT_TIMEOUT        2000
#define HANS_MUTEX_LOCK_TIMEOUT     50
#define HANS_COMAMND_WRITE_TIMEOUT  1000
#define HANS_COMAMND_READ_TIMEOUT   5000

class HansClient : public QThread
{
    Q_OBJECT
public:
    HansClient(QThread::Priority _priority = QThread::NormalPriority);
    ~HansClient();

    void robotConnect(QString address);
    void robotDisconnect();
    bool robotIsConnected();
    void pushCommand(CmdContain cmd);

private:
    void run() override;
    void initClient();
    bool queueCommandIsEmpty();
    void queueCommandClear();
    CmdContain queueCommandGetFront();
    void queueCommandPopFront();
    void commandHandle();

    QByteArray sendCommand(QString cmd);

    void actionConnected();
    void actionDisconnected();

signals:
    void rb_Connected();
    void rb_ConnectFail();
    void rb_Disconnected();

private:
    QThread::Priority threadPriority;
    bool threadRunning;

    QTcpSocket *commandPort;
    QString hostAddress;

    bool hansClientConnected;

    QMutex mutexQueue;
    QList<CmdContain> commandQueue;
};

}
#endif // HANSCLIENT_H
