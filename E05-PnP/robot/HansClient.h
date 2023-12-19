#ifndef HANSCLIENT_H
#define HANSCLIENT_H

#include <chrono>
#include <QThread>
#include <QObject>
#include <QTcpSocket>
#include <QMutex>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueConstRef>
#include <QJsonValue>

#include "HansDefine.h"
#include "HansCommand.h"
#include "time/TimeCounter.h"

using namespace std::chrono;

namespace rb {

class HansClient : public QThread
{
    Q_OBJECT
public:
    HansClient(QThread::Priority _priority = QThread::NormalPriority);
    ~HansClient();

    void robotConnect(QString address);
    void robotDisconnect();
    bool robotIsConnected();
    HansData GetRobotData();
    HansRobotState GetRobotState();
    bool GetRobotBoxDO(int index);
    bool GetRobotBoxDI(int index);
    void pushCommand(CmdContain cmd);
    void RobotStopImmediate();

    /// In-App control functions
    void SetVirtualDI(int index, bool state);
    bool GetVirtualDI(int index);

    /// Optional functions
    void DHGripper_Setup(int input1, int input2, int output1, int output2);
    void DHGripper_Open();
    void DHGripper_Close();
    void DHGripper_Toggle();
    bool DHGripper_IsOpen();
    DhGripperState DHGripper_GetState();

private:
    void run() override;
    void initClient();
    void closeAllConnect();
    bool queueCommandIsEmpty();
    void queueCommandClear();
    CmdContain queueCommandGetFront();
    void queueCommandPopFront();
    void pushRobotQueryInfo();
    void commandHandle();
    void inAppCommandHandle();
    void ImmediateStopHandle();
    void DhGripperStateHandle();

    void responseHandle(QString raw);
    bool responseCommandCheck(QStringList &param);
    void response_ReadRobotState(QStringList &param);
    void response_ReadCurFSM(QStringList &param);
    void response_ReadBoxDI(QStringList &param);
    void response_ReadBoxDO(QStringList &param);

    QByteArray sendCommand(QString cmd);

    /// FEEDBACKS PARSE
    void feedbackFromHostHandle();
    void feedbackParseData(QByteArray &rawBytes);
    int charToUint(char* pBuffer);
    bool intToBool(int value);


    /// EVENT ACTIONS
    void actionCommandDisconnected();
    void actionFeedbackDisconnected();
    void actionConnected();
    void actionDisconnected();

signals:
    void rb_Connected();
    void rb_ConnectFail();
    void rb_Disconnected();
    void rb_RobotStateRefreshed();
    void rb_FeedbackPortReadError();
    void rb_CommandResponseFail(QString cmd);
    void rb_CommandResponseWrongCommand(QString response);
    void rb_CommandResponseWrongFormat(QString response);

    void RbSignal_VirtualDIStrigger(int index, bool state);
    void RbSignal_VirtualDOChange(int index, bool state);
    void RbSignal_BoxDOStrigger(int index, bool state);
    void RbSignal_BoxDIStrigger(int index, bool state);
    void RbSignal_EndDOStrigger(int index, bool state);
    void RbSignal_EndDIStrigger(int index, bool state);
    void RbSignal_StartMove();
    void RbSignal_MoveDone();
    void DhSignal_InStateMoving();
    void DhSignal_InStateHolding();
    void DhSignal_InStateArrived();
    void DhSignal_InStateFail();

  private:
    QThread::Priority threadPriority;
    bool threadRunning;

    QTcpSocket *commandPort;
    QTcpSocket *feedbackPort;
    QString hostAddress;

    bool isRobotConnected;
    ConnectState commandPortState;
    ConnectState feedbackPortState;

    QMutex mutexQueue;
    QList<CmdContain> commandQueue;
    CmdContain lastCommand;

    const char header[4] = {'L', 'T', 'B', 'R'};
    const int headerValue = charToUint((char *)header);

    /// HANS DATA
    HansData robotData;
    bool VirtualDI[8] = { false, false, false, false, false, false, false, false};
    bool VirtualDO[8] = { false, false, false, false, false, false, false, false};

    /// DH-GRIPPER
    DhGripper gripper;
    DhGripperState gripper_previous_state_;

    /// Timer counter
    TimeCounter timeCounter;

    /// Emmergency stop flag
    bool is_immediate_stop_;
};

}
#endif // HANSCLIENT_H
