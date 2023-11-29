#ifndef HANSCOMMAND_H
#define HANSCOMMAND_H

#include <QString>

namespace rb {

struct CmdContain
{
    QString command;

    CmdContain() {
        command = "";
    }

    CmdContain(QString cmd) {
        command = cmd;
    }
};

enum AnalogPattern : int {
    Voltage = 0,
    Current = 1
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

class HansCommand
{
public:
//    HansCommand();
    static CmdContain Electrify();
    static CmdContain BackOut();
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
    static CmdContain SetTCPByName(int robotId, QString name);
    static CmdContain SetUCSByName(int robotId, QString name);
    static CmdContain SetOverride(int robotId, int override);
    static CmdContain SetEndDO(int robotId, int index, bool state);
    static CmdContain SetBoxDO(int index, bool state);
    static CmdContain SetBoxCO(int index, bool state);
    static CmdContain SetBoxAO(int index, double value, AnalogPattern pattern);
    static CmdContain SetBoxAOMode(int index, AnalogPattern pattern);
    static CmdContain ReadEI(int robotId, int index);
    static CmdContain ReadEO(int robotId, int index);
    static CmdContain ReadEAI(int robotId, int index);
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
//    static CmdContain WayPoint(int robotId);
    static CmdContain MoveJ(int robotId, JointPoint point);
    static CmdContain MoveL(int robotId, DescartesPoint point);
};

}

#endif // HANSCOMMAND_H
