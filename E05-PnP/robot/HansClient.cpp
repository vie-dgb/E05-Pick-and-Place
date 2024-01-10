#include "HansClient.h"
#include "HansCommand.h"

namespace rb {

/////// PUBLIC FUNCTIONS
HansClient::HansClient(QThread::Priority _priority)
{
  threadPriority = _priority;
  threadRunning = false;
  isRobotConnected = false;
  is_immediate_stop_ = false;
  continue_trigger_ = false;
  commandPortState = ConnectState::NotConnect;
  feedbackPortState = ConnectState::NotConnect;
}

HansClient::~HansClient() {

}

void HansClient::robotConnect(QString address)
{
  if (!threadRunning) {
    hostAddress = address;
    threadRunning = true;
    start(threadPriority);
  }
}

void HansClient::robotDisconnect()
{
  if (threadRunning) {
    threadRunning = false;
    quit();
    wait();
  }
}

bool HansClient::robotIsConnected()
{
  return isRobotConnected;
}

HansData HansClient::GetRobotData()
{
  return robotData;
}

HansRobotState HansClient::GetRobotState()
{
  return robotData.robotState;
}

bool HansClient::GetRobotBoxDO(int index)
{
  return robotData.BoxDO[index];
}

bool HansClient::GetRobotBoxDI(int index)
{
  return robotData.BoxDI[index];
}

void HansClient::pushCommand(CmdContain cmd)
{
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  commandQueue.push_back(cmd);
  mutexQueue.unlock();
}

void HansClient::pushCommandInFront(CmdContain cmd) {
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  commandQueue.push_front(cmd);
  mutexQueue.unlock();
}

void HansClient::RobotStopImmediate() {
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  is_immediate_stop_ = true;
  mutexQueue.unlock();
}

void HansClient::SetVirtualDI(int index, bool state) {
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  VirtualDI[index] = state;
  mutexQueue.unlock();
}

bool HansClient::GetVirtualDI(int index) {
  bool state = false;
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  state = VirtualDI[index];
  mutexQueue.unlock();
  return state;
}

void HansClient::SetContinueTrigger() {
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  continue_trigger_ = true;
  mutexQueue.unlock();
}

void HansClient::DHGripper_Setup(int input1, int input2, int output1, int output2)
{
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  gripper.input_1 = input1;
  gripper.input_2 = input2;
  gripper.ouput_1 = output1;
  gripper.ouput_2 = output2;
  mutexQueue.unlock();
}

void HansClient::DHGripper_Open()
{
  pushCommand(HansCommand::SetEndDO(0, gripper.ouput_1, false));
  pushCommand(HansCommand::SetEndDO(0, gripper.ouput_2, false));
}

void HansClient::DHGripper_Close()
{
  pushCommand(HansCommand::SetEndDO(0, gripper.ouput_1, true));
}

void HansClient::DHGripper_Toggle()
{
  if (DHGripper_GetState() != DhGripperState::kGripperMoving) {
    if (DHGripper_IsOpen()) {
      DHGripper_Close();
    } else {
      DHGripper_Open();
    }
  }
}

bool HansClient::DHGripper_IsOpen()
{
  return gripper.IsOpen(robotData.EndDO[gripper.ouput_1], robotData.EndDO[gripper.ouput_2]);
}

DhGripperState HansClient::DHGripper_GetState()
{
  return gripper.GetState(robotData.EndDI[gripper.input_1], robotData.EndDI[gripper.input_2]);
}

/////// PRIVATE FUNCTIONS
void HansClient::run()
{
  initClient();

  // close connect when one of them connect fail
  if (!isRobotConnected) {
    closeAllConnect();
    return;
  }

  while (threadRunning) {
    // in case user want stop immediately
    ImmediateStopHandle();

    feedbackFromHostHandle();
    DhGripperStateHandle();
    commandHandle();
  }

  // disconnect all port
  closeAllConnect();
}

void HansClient::initClient()
{
  commandPort = new QTcpSocket;
  feedbackPort = new QTcpSocket;

  // connect action delete client, connected, disconnected
  connect(this, SIGNAL(finished()), commandPort, SLOT(deleteLater()));
  connect(commandPort, &QTcpSocket::disconnected, this, &HansClient::actionCommandDisconnected);
  connect(this, SIGNAL(finished()), feedbackPort, SLOT(deleteLater()));
  connect(feedbackPort, &QTcpSocket::disconnected, this, &HansClient::actionFeedbackDisconnected);

  commandPort->connectToHost(QHostAddress(hostAddress), HANS_COMMAND_PORT);
  if (!commandPort->waitForConnected(HANS_CONNECT_TIMEOUT)) {
    commandPortState = ConnectState::ConnectFail;
  } else {
    commandPortState = ConnectState::Connected;
  }

  feedbackPort->connectToHost(QHostAddress(hostAddress), HANS_FEEDBACK_PORT);
  if (!feedbackPort->waitForConnected(HANS_CONNECT_TIMEOUT)) {
    feedbackPortState = ConnectState::ConnectFail;
  } else {
    feedbackPortState = ConnectState::Connected;
  }

  actionConnected();
  if ((commandPortState == ConnectState::ConnectFail)
      || (feedbackPortState == ConnectState::ConnectFail)) {
    emit rb_ConnectFail();
  }

  queueCommandClear();
}

void HansClient::closeAllConnect()
{
  if (commandPort->state() != QAbstractSocket::UnconnectedState) {
    commandPort->disconnectFromHost();
  }
  if (feedbackPort->state() != QAbstractSocket::UnconnectedState) {
    feedbackPort->disconnectFromHost();
  }
}

bool HansClient::queueCommandIsEmpty()
{
  bool queueIsEmpty = false;
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  queueIsEmpty = commandQueue.isEmpty();
  mutexQueue.unlock();
  return queueIsEmpty;
}

void HansClient::queueCommandClear()
{
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  commandQueue.clear();
  mutexQueue.unlock();
}

CmdContain HansClient::queueCommandGetFront()
{
  CmdContain cmd;
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  cmd = commandQueue.front();
  mutexQueue.unlock();
  return cmd;
}

void HansClient::queueCommandPopFront()
{
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  commandQueue.pop_front();
  mutexQueue.unlock();
}

void HansClient::pushRobotQueryInfo()
{
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  commandQueue.push_back(HansCommand::ReadCurFSM(0));
  commandQueue.push_back(HansCommand::ReadRobotState(0));
  for (int idx = 0; idx < HANS_MAX_DO; idx++) {
    commandQueue.push_back(HansCommand::ReadBoxDO(idx));
  }
  for (int idx = 0; idx < HANS_MAX_DI; idx++) {
    commandQueue.push_back(HansCommand::ReadBoxDI(idx));
  }
  mutexQueue.unlock();
}

void HansClient::commandHandle()
{
  if (queueCommandIsEmpty()) {
    return;
  }

  lastCommand = queueCommandGetFront();

  switch (lastCommand.type) {
    case HansCmdType::Cmd_InApp:
//      qDebug() << "In-app command: " << lastCommand.command;
      inAppCommandHandle();
      break;
    case HansCmdType::Cmd_Remote:
//      qDebug() << "Send command: " << lastCommand.command;
      QString reply = sendCommand(lastCommand.command);
//      qDebug() << "Response command: " << reply;
      responseHandle(reply);
      queueCommandPopFront();
      break;
  }
}

void HansClient::inAppCommandHandle()
{
  HansInAppCmd command = HashInAppCmd(lastCommand.command);

  switch (command) {
    /// Wait until timeout
    case kInApp_WaitTime:
      if (timeCounter.StartTimeCounter(lastCommand.bitIndex)) {
        queueCommandPopFront();
      }
      break;

    /// Wait until VIRTUAL input bit chosen active set state
    case kInApp_WaitVirtualDI:
      if (VirtualDI[lastCommand.bitIndex] == lastCommand.bitState) {
        emit RbSignal_VirtualDIStrigger(lastCommand.bitIndex,
                                        lastCommand.bitState);
        queueCommandPopFront();
      }
      break;

    /// Set state for VIRTUAL output bit when robot isn't moving
    case kInApp_SetVirtualDO:
      if (!robotData.robotState.IsMoving) {
        VirtualDO[lastCommand.bitIndex] = lastCommand.bitState;
        emit RbSignal_VirtualDOChange(lastCommand.bitIndex,
                                      VirtualDO[lastCommand.bitIndex]);
        queueCommandPopFront();
      }
      break;

    /// Wait until BOX output bit chosen active set state
    case kInApp_WaitBoxDO:

      if (robotData.BoxDO[lastCommand.bitIndex] == lastCommand.bitState) {
        emit RbSignal_BoxDOStrigger(lastCommand.bitIndex, lastCommand.bitState);
        queueCommandPopFront();
      }
      break;

    /// Wait until BOX input bit chosen active set state
    case kInApp_WaitBoxDI:
      if (robotData.BoxDI[lastCommand.bitIndex] == lastCommand.bitState) {
        emit RbSignal_BoxDIStrigger(lastCommand.bitIndex, lastCommand.bitState);
        queueCommandPopFront();
      }
      break;

    /// Wait until END output bit chosen active set state
    case kInApp_WaitEndDO:
      if (robotData.EndDO[lastCommand.bitIndex] == lastCommand.bitState) {
        emit RbSignal_EndDOStrigger(lastCommand.bitIndex, lastCommand.bitState);
        queueCommandPopFront();
      }
      break;

    /// Wait until END input bit chosen active set state
    case kInApp_WaitEndDI:
      if (robotData.EndDI[lastCommand.bitIndex] == lastCommand.bitState) {
        emit RbSignal_EndDIStrigger(lastCommand.bitIndex, lastCommand.bitState);
        queueCommandPopFront();
      }
      break;

    /// Wait until robot in STATE different with moving
    case kInApp_WaitMoveDone:
      if (!robotData.robotState.IsMoving) {
        emit RbSignal_MoveDone(lastCommand.bitIndex);
        queueCommandPopFront();
      }
      break;

    /// Wait until robot in STATE moving
    case kInApp_WaitStartMove:
      if (robotData.robotState.IsMoving) {
        emit RbSignal_StartMove(lastCommand.bitIndex);
        queueCommandPopFront();
      }
      break;

    /// Trigger output int value
    case kInApp_TriggerOutputInt:
      emit RbSignal_TriggeredOutputInt(lastCommand.bitIndex);
      queueCommandPopFront();
      break;

    /// Wait until robot in STATE moving
    case kInApp_WaitContinueTrigger:
      if (continue_trigger_) {
        continue_trigger_ = false;
        queueCommandPopFront();
      }
      break;

    /// Wait hd gripper in state holding
    case kInApp_WaitDhGripperHolding:
      if (gripper.last_state == DhGripperState::kGripperHolding) {
        queueCommandPopFront();
      }
      break;

    /// Wait hd gripper in state holding
    case kInApp_WaitDhGripperArrived:
      if (gripper.last_state == DhGripperState::kGripperArrived) {
        queueCommandPopFront();
      }
      break;
  }
}

void HansClient::ImmediateStopHandle() {
  // lock all memories inside thread until robot stop
  mutexQueue.tryLock(HANS_MUTEX_LOCK_TIMEOUT);
  if(is_immediate_stop_) {
//    CmdContain stop_command = HansCommand::GrpStop(0);
//    qDebug() << "Send command: " << stop_command.command;
//    QString reply = sendCommand(stop_command.command);
//    qDebug() << "Response command: " << reply;
//    queueCommandClear();
    queueCommandClear();
    pushCommand(HansCommand::GrpStop(0));
    if(robotData.robotState.IsPowerOn) {
      pushCommand(HansCommand::GrpPowerOff(0));
    }
//    pushCommand(HansCommand::BlackOut());
    is_immediate_stop_ = false;
  }
  mutexQueue.unlock();
}

void HansClient::DhGripperStateHandle() {
  DHGripper_GetState();
  if(gripper_previous_state_ != gripper.last_state) {
    gripper_previous_state_ = gripper.last_state;
    switch (gripper_previous_state_) {
      case DhGripperState::kGripperArrived:
        emit DhSignal_InStateArrived();
        break;
      case DhGripperState::kGripperMoving:
        emit DhSignal_InStateMoving();
        break;
      case DhGripperState::kGripperHolding:
        emit DhSignal_InStateHolding();
        break;
      case DhGripperState::kGripperFail:
        emit DhSignal_InStateFail();
        break;
    }
  }
}

void HansClient::responseHandle(QString raw)
{
  // remove ",;"
  if (!(raw.right(2) == ",;")) {
    emit rb_CommandResponseWrongFormat(raw);
    return;
  }

  QString rawCutEnd = raw.left(raw.length() - 2);
  QStringList parsed = rawCutEnd.split(',');

  if (parsed[1].toLower() == "ok") {
    if (!responseCommandCheck(parsed)) {
      emit rb_CommandResponseWrongCommand(raw);
    }
  } else if (parsed[1].toLower() == "fail") {
    emit rb_CommandResponseFail(lastCommand.command);
  } else {
    emit rb_CommandResponseWrongFormat(raw);
  }
}

bool HansClient::responseCommandCheck(QStringList &param)
{
  if (param[0] == CMD_ReadRobotState) {
    response_ReadRobotState(param);
    emit rb_RobotStateRefreshed();
  } else if (param[0] == CMD_ReadCurFSM) {
    response_ReadCurFSM(param);
  } else if (param[0] == CMD_ReadBoxDI) {
    response_ReadBoxDI(param);
  } else if (param[0] == CMD_ReadBoxDO) {
    response_ReadBoxDO(param);
  }
  return true;
}

void HansClient::response_ReadRobotState(QStringList &param)
{
  robotData.robotState.IsMoving = (param[2] == "0") ? false : true;
  robotData.robotState.IsPowerOn = (param[3] == "0") ? false : true;
  robotData.robotState.IsError = (param[4] == "0") ? false : true;
  robotData.robotState.ErrorCode = param[5].toInt();
  robotData.robotState.ErrorAxisID = param[6].toInt();
  robotData.robotState.IsBraking = (param[7] == "0") ? false : true;
  robotData.robotState.IsHolding = (param[8] == "0") ? false : true;
  robotData.robotState.IsEmerStopping = (param[9] == "0") ? false : true;
  robotData.robotState.IsSafetyGuardOperate = (param[10] == "0") ? false : true;
  robotData.robotState.ElectrifyState = (param[11] == "0") ? false : true;
  robotData.robotState.IsConnectToBox = (param[12] == "0") ? false : true;
  robotData.robotState.IsBlendingDone = (param[13] == "0") ? false : true;
  robotData.robotState.IsInPosition = (param[14] == "0") ? false : true;
}

void HansClient::response_ReadCurFSM(QStringList &param)
{
  robotData.robotState.MachineState = static_cast<HansMachineState>(param[2].toInt());
}

void HansClient::response_ReadBoxDI(QStringList &param)
{
  robotData.BoxDI[lastCommand.bitIndex] = (param[2] == "0") ? false : true;
}

void HansClient::response_ReadBoxDO(QStringList &param)
{
  robotData.BoxDO[lastCommand.bitIndex] = (param[2] == "0") ? false : true;
}

QByteArray HansClient::sendCommand(QString cmd)
{
  commandPort->write(cmd.toUtf8());
  commandPort->waitForBytesWritten(HANS_COMAMND_WRITE_TIMEOUT);
  commandPort->waitForReadyRead(HANS_COMAMND_RESPONSE_TIMEOUT);
  return commandPort->readAll();
}

void HansClient::feedbackFromHostHandle()
{
  // check feedback timeout or lost connect suddenly
  if (!feedbackPort->waitForReadyRead(500)) {
    emit rb_FeedbackPortReadError();
    threadRunning = false;
    queueCommandClear();
    return;
  }

  // read all byte available on buffer
  QByteArray raw = feedbackPort->readAll();
  QByteArray dataSheet;
  if (raw.isEmpty()) {
    return;
  }

  char *pBuffer = raw.data();
  // get newest data, so check from tail to head
  for (int index = raw.size() - 13; index >= 0; index--) {
    int headerCheck = charToUint(pBuffer + index);
    if (headerCheck == headerValue) {
      int total_Size = charToUint(pBuffer + index + 4);
      if ((index + total_Size) > raw.size()) {
        continue;
      }
      int data_Size = charToUint(pBuffer + index + 8);
      // just for sure received correct format
      if ((total_Size - data_Size) == 12) {
        dataSheet = raw.mid(index + 12, data_Size);
        feedbackParseData(dataSheet);
        break;
      }
    }
  }
}

void HansClient::feedbackParseData(QByteArray &rawBytes)
{
  const QJsonDocument jsonDoc = QJsonDocument::fromJson(rawBytes);
  const QJsonValue EndIO = jsonDoc["EndIO"];
  const QJsonValue ElectricBoxIO = jsonDoc["ElectricBoxIO"];
  const QJsonValue PosAndVel = jsonDoc["PosAndVel"];
  const QJsonValue StateAndError = jsonDoc["StateAndError"];

  for (int index = 0; index < HANS_MAX_END_DO; index++) {
    robotData.EndDO[index] = intToBool(EndIO["EndDO"][index].toInt());
    robotData.EndDI[index] = intToBool(EndIO["EndDI"][index].toInt());
  }

  for (int index = 0; index < HANS_MAX_DO; index++) {
    robotData.BoxDO[index] = intToBool(ElectricBoxIO["BoxDO"][index].toInt());
    robotData.BoxDI[index] = intToBool(ElectricBoxIO["BoxDI"][index].toInt());
  }

  robotData.ActualOverride = PosAndVel["Actual_Override"][0].toDouble();

  robotData.ActualJoint.J1 = PosAndVel["Actual_Position"][0].toDouble();
  robotData.ActualJoint.J2 = PosAndVel["Actual_Position"][1].toDouble();
  robotData.ActualJoint.J3 = PosAndVel["Actual_Position"][2].toDouble();
  robotData.ActualJoint.J4 = PosAndVel["Actual_Position"][3].toDouble();
  robotData.ActualJoint.J5 = PosAndVel["Actual_Position"][4].toDouble();
  robotData.ActualJoint.J6 = PosAndVel["Actual_Position"][5].toDouble();

  robotData.ActualPosition.X = PosAndVel["Actual_Position"][6].toDouble();
  robotData.ActualPosition.Y = PosAndVel["Actual_Position"][7].toDouble();
  robotData.ActualPosition.Z = PosAndVel["Actual_Position"][8].toDouble();
  robotData.ActualPosition.rX = PosAndVel["Actual_Position"][9].toDouble();
  robotData.ActualPosition.rY = PosAndVel["Actual_Position"][10].toDouble();
  robotData.ActualPosition.rZ = PosAndVel["Actual_Position"][11].toDouble();

  robotData.robotState.MachineState = static_cast<HansMachineState>(
      StateAndError["robotState"].toInt());
  robotData.robotState.IsMoving = intToBool(StateAndError["robotMoving"].toInt());
  robotData.robotState.IsPowerOn = intToBool(StateAndError["robotEnabled"].toInt());
  robotData.robotState.ErrorCode = StateAndError["Error_Code"].toInt();
  if (robotData.robotState.ErrorCode != 0) {
    robotData.robotState.IsError = (robotData.robotState.ErrorCode != 0) ? true : false;
  }
  robotData.robotState.ErrorAxisID = StateAndError["Error_AxisID"].toInt();
  robotData.robotState.IsBraking = intToBool(StateAndError["robotEnabled"][0].toInt());
  //    robotData.robotState.IsHolding = StateAndError["robotEnabled"].toBool();
  robotData.robotState.IsEmerStopping = (robotData.robotState.MachineState
                                         == HansMachineState::EmergencyStop)
                                            ? true
                                            : false;
  robotData.robotState.IsSafetyGuardOperate = intToBool(StateAndError["robotEnabled"].toInt());
  //    robotData.robotState.ElectrifyState = StateAndError["robotEnabled"].toBool();
  //    robotData.robotState.IsConnectToBox = StateAndError["robotEnabled"].toBool();
  robotData.robotState.IsBlendingDone = intToBool(StateAndError["robotBlendingDone"].toInt());
  robotData.robotState.IsInPosition = intToBool(StateAndError["InPos"].toInt());
}

int HansClient::charToUint(char *pBuffer)
{
  int value = pBuffer[3] & 0xFF;
  value <<= 8;
  value |= pBuffer[2] & 0xFF;
  value <<= 8;
  value |= pBuffer[1] & 0xFF;
  value <<= 8;
  value |= pBuffer[0] & 0xFF;
  return value;
}

bool HansClient::intToBool(int value)
{
  return (value == 0) ? false : true;
}

//////// SLOT ACTIONS

void HansClient::actionCommandDisconnected()
{
  commandPortState = ConnectState::NotConnect;
  actionDisconnected();
}

void HansClient::actionFeedbackDisconnected()
{
  feedbackPortState = ConnectState::NotConnect;
  actionDisconnected();
}

void HansClient::actionConnected()
{
  if ((commandPortState == ConnectState::Connected)
      && (feedbackPortState == ConnectState::Connected)) {
    isRobotConnected = true;
    emit rb_Connected();
  }
}

void HansClient::actionDisconnected()
{
  isRobotConnected = false;
  if ((commandPortState == ConnectState::NotConnect)
      && (feedbackPortState == ConnectState::NotConnect)) {
    emit rb_Disconnected();
  }
}
}
