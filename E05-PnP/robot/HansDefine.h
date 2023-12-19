#ifndef HANSDEFINE_H
#define HANSDEFINE_H

#include <QString>

#define HANS_COMMAND_PORT               10003
#define HANS_FEEDBACK_PORT              10004
#define HANS_CONNECT_TIMEOUT            2000
#define HANS_MUTEX_LOCK_TIMEOUT         50
#define HANS_COMAMND_WRITE_TIMEOUT      200
#define HANS_COMAMND_RESPONSE_TIMEOUT   1000
#define HANS_MAX_END_DO                 4
#define HANS_MAX_END_DI                 8
#define HANS_MAX_DO                     8
#define HANS_MAX_DI                     8

/// STRING COMMAND
#define CMD_ReadRobotState          "ReadRobotState"
#define CMD_ReadCurFSM              "ReadCurFSM"
#define CMD_ReadBoxDI               "ReadBoxDI"
#define CMD_ReadBoxDO               "ReadBoxDO"

#define CMD_WaitTime                "WaitTime"
#define CMD_WaitVirtualDI           "WaitVirtualDI"
#define CMD_WaitBoxDO               "WaitBoxDO"
#define CMD_WaitBoxDI               "WaitBoxDI"
#define CMD_WaitEndDO               "WaitEndDO"
#define CMD_WaitEndDI               "WaitEndDI"
#define CMD_WaitMoveDone            "WaitMoveDone"
#define CMD_WaitStartMove           "WaitStartMove"
#define CMD_SetVirtualDO            "SetVirtualDO"
#define CMD_WaitDhGripper           "WaitDhGripper"
#define CMD_WaitDhGripperHolding    "WaitDhGripperHolding"
#define CMD_WaitDhGripperArrived    "WaitDhGripperArrived"

namespace rb {

enum ConnectState : int {
  NotConnect = 0,
  Connected,
  ConnectFail
};

enum AnalogPattern : int {
  Voltage = 0,
  Current = 1
};

enum MoveType : int {
  MoveJ = 0,
  MoveL = 1
};

enum MoveDirection : int {
  Negative = 0,
  Positive = 1
};

enum JointID : int {
  J1 = 0,
  J2 = 1,
  J3 = 2,
  J4 = 3,
  J5 = 4,
  J6 = 5
};

enum AxisID : int {
  X = 0,
  Y = 1,
  Z = 2,
  RX = 3,
  RY = 4,
  RZ = 5
};

enum HansMachineState : int {
  UnInitialize = 0,
  Initialize,
  ElectricBoxDisconnect,
  ElectricBoxConnecting,
  EmergencyStopHandling,
  EmergencyStop,
  Blackouting48V,
  Blackout_48V,
  Electrifying48V,
  SafetyGuardErrorHandling,
  SafetyGuardError,
  SafetyGuardHandling,
  SafetyGuard,
  ControllerDisconnecting,
  ControllerDisconnect,
  ControllerConnecting,
  ControllerVersionError,
  EtherCATError,
  ControllerChecking,
  Reseting,
  RobotOutOfSafeSpace,
  RobotCollisionStop,
  Error,
  RobotEnabling,
  Disable,
  Moving,
  LongJogMoving,
  RobotStopping,
  RobotDisabling,
  RobotOpeningFreeDriver,
  RobotClosingFreeDriver,
  FreeDriver,
  RobotHolding,
  StandBy,
  ScriptRunning,
  ScriptHoldHandling,
  ScriptHolding,
  ScriptStopping,
  ScriptStopped,
  HRAppDisconnected,
  HRAppError
};

struct HansRobotState
{
  bool IsMoving;
  bool IsPowerOn;
  bool IsError;
  int ErrorCode;
  int ErrorAxisID;
  bool IsBraking;
  bool IsHolding;
  bool IsEmerStopping;
  bool IsSafetyGuardOperate;
  bool ElectrifyState;
  bool IsConnectToBox;
  bool IsBlendingDone;
  bool IsInPosition;
  HansMachineState MachineState;

  HansRobotState() {
    IsMoving = false;
    IsPowerOn = false;
    IsError = false;
    ErrorCode = 0;
    ErrorAxisID = 0;
    IsBraking = false;
    IsHolding = false;
    IsEmerStopping = false;
    IsSafetyGuardOperate = false;
    ElectrifyState = false;
    IsConnectToBox = false;
    IsBlendingDone = false;
    IsInPosition = false;
    MachineState = UnInitialize;
  }

  QString MachineStateToQString() {
    switch(MachineState) {
      case HansMachineState::UnInitialize:
        return "UnInitialize";
      case HansMachineState::Initialize:
        return "Initialize";
      case HansMachineState::ElectricBoxDisconnect:
        return "Electric Box Disconnect";
      case HansMachineState::ElectricBoxConnecting:
        return "Electric Box Connecting";
      case HansMachineState::EmergencyStopHandling:
        return "Emergency Stop Handling";
      case HansMachineState::EmergencyStop:
        return "Emergency Stop";
      case HansMachineState::Blackouting48V:
        return "Blackouting 48V";
      case HansMachineState::Blackout_48V:
        return "Blackout 48V";
      case HansMachineState::Electrifying48V:
        return "Electrifying 48V";
      case HansMachineState::SafetyGuardErrorHandling:
        return "Safety Guard Error Handling";
      case HansMachineState::SafetyGuardError:
        return "Safety Guard Error";
      case HansMachineState::SafetyGuardHandling:
        return "Safety Guard Handling";
      case HansMachineState::SafetyGuard:
        return "Safety Guard";
      case HansMachineState::ControllerDisconnecting:
        return "Controller Disconnecting";
      case HansMachineState::ControllerDisconnect:
        return "Controller Disconnect";
      case HansMachineState::ControllerConnecting:
        return "Controller Connecting";
      case HansMachineState::ControllerVersionError:
        return "Controller Version Error";
      case HansMachineState::EtherCATError:
        return "EtherCAT Error";
      case HansMachineState::ControllerChecking:
        return "Controller Checking";
      case HansMachineState::Reseting:
        return "Reseting";
      case HansMachineState::RobotOutOfSafeSpace:
        return "Robot Out Of Safe Space";
      case HansMachineState::RobotCollisionStop:
        return "Robot Collision Stop";
      case HansMachineState::Error:
        return "Error";
      case HansMachineState::RobotEnabling:
        return "Robot Enabling";
      case HansMachineState::Disable:
        return "Disable";
      case HansMachineState::Moving:
        return "Moving";
      case HansMachineState::LongJogMoving:
        return "Long Jog Moving";
      case HansMachineState::RobotStopping:
        return "Robot Stopping";
      case HansMachineState::RobotDisabling:
        return "Robot Disabling";
      case HansMachineState::RobotOpeningFreeDriver:
        return "Robot Opening Free Driver";
      case HansMachineState::RobotClosingFreeDriver:
        return "Robot Closing Free Driver";
      case HansMachineState::FreeDriver:
        return "Free Driver";
      case HansMachineState::RobotHolding:
        return "Robot Holding";
      case HansMachineState::StandBy:
        return "StandBy";
      case HansMachineState::ScriptRunning:
        return "Script Running";
      case HansMachineState::ScriptHoldHandling:
        return "Script Hold Handling";
      case HansMachineState::ScriptHolding:
        return "Script Holding";
      case HansMachineState::ScriptStopping:
        return "Script Stopping";
      case HansMachineState::ScriptStopped:
        return "Script Stopped";
      case HansMachineState::HRAppDisconnected:
        return "HRApp Disconnected";
      case HansMachineState::HRAppError:
        return "HRApp Error";
    }
    return "";
  }
};

enum HansCmdType : int {
  Cmd_InApp = 0,
  Cmd_Remote
};

struct CmdContain
{
  HansCmdType type = HansCmdType::Cmd_Remote;
  QString command = "";
  int bitIndex = 0;
  bool bitState = false;

  CmdContain() {}

  CmdContain(HansCmdType cmdType, QString cmd) {
    type = cmdType;
    command = cmd;
  }

  CmdContain(HansCmdType cmdType, QString cmd, int _bitIndex) {
    type = cmdType;
    command = cmd;
    bitIndex = _bitIndex;
  }

  CmdContain(HansCmdType cmdType, QString cmd, int _bitIndex, bool _bitState) {
    type = cmdType;
    command = cmd;
    bitIndex = _bitIndex;
    bitState = _bitState;
  }
};

struct DescartesPoint
{
  double X = 0.0;
  double Y = 0.0;
  double Z = 0.0;
  double rX = 0.0;
  double rY = 0.0;
  double rZ = 0.0;
  QString plane = "Base";
  QString tcp = "TCP";

  DescartesPoint() {}

  DescartesPoint(double x, double y, double z,
                 double rx, double ry, double rz) {
    X = x;
    Y = y;
    Z = z;
    rX = rx;
    rY = ry;
    rZ = rz;
  }

  DescartesPoint(double x, double y, double z,
                 double rx, double ry, double rz,
                 QString plane_name, QString tcp_name) {
    X = x;
    Y = y;
    Z = z;
    rX = rx;
    rY = ry;
    rZ = rz;
    plane = plane_name;
    tcp = tcp_name;
  }
};

struct JointPoint
{
  double J1 = 0.0;
  double J2 = 0.0;
  double J3 = 0.0;
  double J4 = 0.0;
  double J5 = 0.0;
  double J6 = 0.0;
  QString tcp = "TCP";

  JointPoint() {}

  JointPoint(double j1, double j2, double j3,
             double j4, double j5, double j6,
             QString tcp_name) {
    J1 = j1;
    J2 = j2;
    J3 = j3;
    J4 = j4;
    J5 = j5;
    J6 = j6;
    tcp = tcp_name;
  }
};

struct HansData
{
  DescartesPoint ActualPosition;
  JointPoint ActualJoint;
  DescartesPoint ActualPCS_Base;
  DescartesPoint ActualPCS_TCP;
  double ActualOverride;

  bool EndDO[HANS_MAX_END_DO];
  bool EndDI[HANS_MAX_END_DI];

  bool BoxDO[HANS_MAX_DO];
  bool BoxDI[HANS_MAX_DI];

  HansRobotState robotState;

  HansData() {}
};


enum DhGripperState : int {
  kGripperMoving = 0,
  kGripperArrived,
  kGripperHolding,
  kGripperFail
};

static QString DhGripStateToQString(DhGripperState state) {
  switch (state) {
    case DhGripperState::kGripperMoving:
      return "In motion";
    case DhGripperState::kGripperArrived:
      return "In reference position";
    case DhGripperState::kGripperHolding:
      return "Holding object";
    case DhGripperState::kGripperFail:
      return "Object holding fail";
  }
  return "";
}

struct DhGripper
{
  int ouput_1 = 0;
  int ouput_2 = 0;
  int input_1 = 0;
  int input_2 = 0;
  DhGripperState last_state = DhGripperState::kGripperArrived;

  DhGripper() {}

  bool IsOpen(bool state_out1, bool state_out2) {
    if((!state_out1) && (!state_out2)) {
      return true;
    }
    return false;
  }

  DhGripperState GetState(bool state_in1, bool state_in2) {
    if((state_in1) && (!state_in2)) {
      last_state = DhGripperState::kGripperArrived;
    }
    else if((!state_in1) && (state_in2)) {
      last_state = DhGripperState::kGripperHolding;
    }
    else if(state_in1 && state_in2) {
      last_state = DhGripperState::kGripperFail;
    }
    else {
      last_state = DhGripperState::kGripperMoving;
    }
    return last_state;
  }

  QString ToString() {
    return DhGripStateToQString(last_state);
  }
};

}

#endif // HANSDEFINE_H
