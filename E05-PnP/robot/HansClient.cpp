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

HansRobotState HansClient::GetRobotState() {
    return robotState;
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

    TimeCounter cyclicTimeCounter(100);
    cyclicTimeCounter.MarkStartPoint();

    while(threadRunning) {
        // send request read robot state
        if(cyclicTimeCounter.TimeOutCheckingCylic()) {
            pushCommand(HansCommand::ReadCurFSM(0));
            pushCommand(HansCommand::ReadRobotState(0));
        }

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

    lastCommand = queueCommandGetFront();
    QString reply = sendCommand(lastCommand.command);
    responseHandle(reply);
    qDebug() << reply;
    queueCommandPopFront();
}

void HansClient::responseHandle(QString raw) {
    // remove ",;"
    if(!(raw.right(2) == ",;")) {
        emit rb_CommandResponseWrongFormat(raw);
        return;
    }

    QString rawCutEnd = raw.left(raw.length() - 2);
    QStringList parsed = rawCutEnd.split(',');

    if(parsed[1].toLower() == "ok") {
        if(!responseCommandCheck(parsed)) {
            emit rb_CommandResponseWrongCommand(raw);
        }
    }
    else if(parsed[1].toLower() == "fail") {
        emit rb_CommandResponseFail(lastCommand.command);
    }
    else {
        emit rb_CommandResponseWrongFormat(raw);
    }
}

bool HansClient::responseCommandCheck(QStringList &param) {
    if(param[0] == CMD_ReadRobotState) {
        response_ReadRobotState(param);
        emit rb_RobotStateRefreshed();
    }
    else if(param[0] == CMD_ReadCurFSM) {
        response_ReadCurFSM(param);
    }
    return true;
}

void HansClient::response_ReadRobotState(QStringList &param) {
    robotState.IsMoving = (param[2] == "0") ? false : true;
    robotState.IsPowerOn = (param[3] == "0") ? false : true;
    robotState.IsError = (param[4] == "0") ? false : true;
    robotState.ErrorCode = param[5].toInt();
    robotState.ErrorAxisID = param[6].toInt();
    robotState.IsBraking = (param[7] == "0") ? false : true;
    robotState.IsHolding = (param[8] == "0") ? false : true;
    robotState.IsEmerStopping = (param[9] == "0") ? false : true;
    robotState.IsSafetyGuardOperate = (param[10] == "0") ? false : true;
    robotState.ElectrifyState = (param[11] == "0") ? false : true;
    robotState.IsConnectToBox = (param[12] == "0") ? false : true;
    robotState.IsBlendingDone = (param[13] == "0") ? false : true;
    robotState.IsInPosition = (param[14] == "0") ? false : true;
}

void HansClient::response_ReadCurFSM(QStringList &param) {
    robotState.MachineState = static_cast<HansMachineState>(param[2].toInt());
}

QByteArray HansClient::sendCommand(QString cmd) {
//    qDebug() << "Send command: " << cmd;
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
