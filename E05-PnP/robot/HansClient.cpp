#include "HansClient.h"
#include "HansCommand.h"

namespace rb {

HansClient::HansClient(QThread::Priority _priority) {
    threadPriority = _priority;
    threadRunning = false;
    hansClientConnected = false;
}

HansClient::~HansClient() {

}

void HansClient::robotConnect(QString address) {
    if(!threadRunning) {
        hostAddress = address;
        threadRunning = true;
        start(threadPriority);
    }
}

void HansClient::robotDisconnect() {
    if(threadRunning) {
        threadRunning = false;
        quit();
        wait();
    }
}

bool HansClient::robotIsConnected() {
    return hansClientConnected;
}

void HansClient::pushCommand(CmdContain cmd) {
    mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
    commandQueue.push_back(cmd);
    mutexQueue.unlock();
}

void HansClient::run() {
    initClient();

    if(!hansClientConnected) {
        return;
    }

    while(threadRunning) {
        commandHandle();
    }
}

void HansClient::initClient() {
    commandPort = new QTcpSocket;

    // connect action delete client, connected, disconnected
    connect(this, SIGNAL(finished()), commandPort, SLOT(deleteLater()));
    connect(commandPort, &QTcpSocket::connected, this, &HansClient::actionConnected);
    connect(commandPort, &QTcpSocket::disconnected, this, &HansClient::actionDisconnected);

    commandPort->connectToHost(QHostAddress(hostAddress), HANS_COMMAND_PORT);
    if(!commandPort->waitForConnected(HANS_CONNECT_TIMEOUT)) {
        emit rb_ConnectFail();
        hansClientConnected = false;
    } else {
        hansClientConnected = true;
    }

    queueCommandClear();
}

bool HansClient::queueCommandIsEmpty() {
    bool queueIsEmpty = false;
    mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
    queueIsEmpty = commandQueue.isEmpty();
    mutexQueue.unlock();
    return queueIsEmpty;
}

void HansClient::queueCommandClear() {
    mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
    commandQueue.clear();
    mutexQueue.unlock();
}

CmdContain HansClient::queueCommandGetFront() {
    CmdContain cmd;
    mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
    cmd = commandQueue.front();
    mutexQueue.unlock();
    return cmd;
}

void HansClient::queueCommandPopFront() {
    mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
    commandQueue.pop_front();
    mutexQueue.unlock();
}

void HansClient::commandHandle() {
    if(queueCommandIsEmpty()) {
        return;
    }

    CmdContain cmd = queueCommandGetFront();
    qDebug() << sendCommand(cmd.command);
    queueCommandPopFront();
}

QByteArray HansClient::sendCommand(QString cmd) {
    qDebug() << "Send command: " << cmd;
    commandPort->write(cmd.toUtf8());
    commandPort->waitForBytesWritten(HANS_COMAMND_WRITE_TIMEOUT);
    commandPort->waitForReadyRead(HANS_COMAMND_READ_TIMEOUT);
    return commandPort->readAll();
}

void HansClient::actionConnected() {
    emit rb_Connected();
}

void HansClient::actionDisconnected() {
    threadRunning = false;
    emit rb_Disconnected();
    hansClientConnected = false;
    quit();
    wait();
}

}
