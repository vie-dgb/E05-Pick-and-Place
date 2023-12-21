#ifndef HANSCOMMAND_H
#define HANSCOMMAND_H

#include <QString>

#include "HansDefine.h"

namespace rb {

class HansCommand
{
public:
//    HansCommand();
    //////// IN-APP COMMAND
    static CmdContain WaitTime(int timeout);
    static CmdContain WaitVirtualDI(int index, bool state);
    static CmdContain SetVirtualDO(int index, bool state);
    static CmdContain WaitBoxDO(int index, bool state);
    static CmdContain WaitBoxDI(int index, bool state);
    static CmdContain WaitEndDO(int index, bool state);
    static CmdContain WaitEndDI(int index, bool state);
    static CmdContain WaitMoveDone(int result);
    static CmdContain WaitStartMove(int result);
    static CmdContain TriggerOutputInt(int value);
    static CmdContain WaitContinueTrigger();
    static CmdContain WaitDhGripperHolding();
    static CmdContain WaitDhGripperArrived();

    //////// REMOTE COMMAND
    static CmdContain Electrify();
    static CmdContain BlackOut();
    static CmdContain StartMaster();
    static CmdContain ClostMaster();
    static CmdContain GrpPowerOn(int robotId);
    static CmdContain GrpPowerOff(int robotId);
    static CmdContain GrpStop(int robotId);
    static CmdContain GrpReset(int robotId);
    static CmdContain GrpInterrupt(int robotId);
    static CmdContain GrpContinue(int robotId);
    static CmdContain GrpOpenFreeDriver(int robotId);
    static CmdContain GrpCloseFreeDriver(int robotId);
    static CmdContain SetTCPByName(int robotId,
                                   QString name);
    static CmdContain SetUCSByName(int robotId,
                                   QString name);
    static CmdContain SetOverride(int robotId,
                                  int override);
    static CmdContain SetEndDO(int robotId,
                               int index,
                               bool state);
    static CmdContain SetBoxDO(int index, bool state);
    static CmdContain SetBoxCO(int index, bool state);
    static CmdContain SetBoxAO(int index,
                               double value,
                               AnalogPattern pattern);
    static CmdContain SetBoxAOMode(int index,
                                   AnalogPattern pattern);
    static CmdContain ReadEI(int robotId,
                             int index);
    static CmdContain ReadEO(int robotId,
                             int index);
    static CmdContain ReadEAI(int robotId,
                              int index);
    static CmdContain ReadBoxDI(int index);
    static CmdContain ReadBoxCI(int index);
    static CmdContain ReadBoxCO(int index);
    static CmdContain ReadBoxDO(int index);
    static CmdContain ReadBoxAO(int index);
    static CmdContain ReadBoxAI(int index);
    static CmdContain ReadActPos(int robotId);
    static CmdContain ReadOverride(int robotId);
    static CmdContain ReadRobotState(int robotId);
    static CmdContain ReadCurFSM(int robotId);
//    static CmdContain PCS2ACS(int robotId);
    static CmdContain WayPoint(int robotId,
                               DescartesPoint PCS,
                               JointPoint ACS,
                               QString TCPName,
                               QString UCSName,
                               double velo,
                               double Accel,
                               double Radius,
                               MoveType type,
                               bool isUseJoint,
                               bool isSeek,
                               int bit,
                               bool state,
                               QString PointGuid = " ");
    static CmdContain MoveJ(int robotId,
                            JointPoint point);
    static CmdContain MoveL(int robotId,
                            DescartesPoint point);
    static CmdContain ShortJogJ(int robotId,
                                JointID joint,
                                MoveDirection direction);
    static CmdContain ShortJogL(int robotId,
                                AxisID axis,
                                MoveDirection direction);
    static CmdContain PauseScript();
    static CmdContain ContinueScript();
    static CmdContain StartScript();
    static CmdContain StopScript();
//    static CmdContain RunFunc();

    static CmdContain WayPointL(int robotId,
                                DescartesPoint PCS,
                                double velo,
                                double Accel,
                                double Radius);
    static CmdContain WayPointLRelRef(int robotId,
                                DescartesPoint PCS,
                                double x,
                                double y,
                                double z,
                                double rz,
                                double velo,
                                double Accel,
                                double Radius);
//    static CmdContain WayPointJ_Joint(int robotId, JointPoint ACS,
//        double velo, double Accel, double Radius);
};

}

#endif // HANSCOMMAND_H
