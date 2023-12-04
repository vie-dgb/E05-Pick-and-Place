#ifndef HANSDEFINE_H
#define HANSDEFINE_H

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

struct CmdContain
{
    QString command = "";
    int bitIndex = 0;

    CmdContain() {}

    CmdContain(QString cmd) {
        command = cmd;
    }

    CmdContain(QString cmd, int _bitIndex) {
        command = cmd;
        bitIndex = _bitIndex;
    }
};

struct DescartesPoint
{
    double X;
    double Y;
    double Z;
    double rX;
    double rY;
    double rZ;
    DescartesPoint() {
        X = 0.0;
        Y = 0.0;
        Z = 0.0;
        rX = 0.0;
        rY = 0.0;
        rZ = 0.0;
    }

    DescartesPoint(double x, double y, double z, double rx, double ry, double rz) {
        X = x;
        Y = y;
        Z = z;
        rX = rx;
        rY = ry;
        rZ = rz;
    }
};

struct JointPoint
{
    double J1;
    double J2;
    double J3;
    double J4;
    double J5;
    double J6;
    JointPoint() {
        J1 = 0.0;
        J2 = 0.0;
        J3 = 0.0;
        J4 = 0.0;
        J5 = 0.0;
        J6 = 0.0;
    }

    JointPoint(double j1, double j2, double j3, double j4, double j5, double j6) {
        J1 = j1;
        J2 = j2;
        J3 = j3;
        J4 = j4;
        J5 = j5;
        J6 = j6;
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


enum DH_GripperState : int {
    Gripper_Moving = 0,
    Gripper_Arrived,
    Gripper_Catched,
    Gripper_Dropped
};

struct DH_Gripper
{
    int Ouput_1 = 0;
    int Ouput_2 = 0;
    int Input_1 = 0;
    int Input_2 = 0;
    DH_GripperState lastState = DH_GripperState::Gripper_Arrived;

    DH_Gripper() {}
};

}

#endif // HANSDEFINE_H
