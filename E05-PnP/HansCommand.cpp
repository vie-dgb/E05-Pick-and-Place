#include "HansCommand.h"

namespace rb {

CmdContain HansCommand::Electrify() {
    return CmdContain("Electrify,;");
}

CmdContain HansCommand::BackOut() {
    return CmdContain("BlackOut,;");
}

CmdContain HansCommand::StartMaster() {
    return CmdContain("StartMaster,;");
}

CmdContain HansCommand::ClostMaster() {
    return CmdContain("CloseMaster,;");
}

CmdContain HansCommand::GrpPowerOn(int robotId) {
    QString cmd = QString::asprintf("GrpPowerOn,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpPowerOff(int robotId) {
    QString cmd = QString::asprintf("GrpPowerOff,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpStop(int robotId) {
    QString cmd = QString::asprintf("GrpStop,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpReset(int robotId) {
    QString cmd = QString::asprintf("GrpReset,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpInterrupt(int robotId) {
    QString cmd = QString::asprintf("GrpInterrupt,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpContinue(int robotId) {
    QString cmd = QString::asprintf("GrpContinue,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpOpenFreeDriver(int robotId) {
    QString cmd = QString::asprintf("GrpOpenFreeDriver,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::GrpCloseFreeDriver(int robotId) {
    QString cmd = QString::asprintf("GrpCloseFreeDriver,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::SetTCPByName(int robotId, QString name) {
    QString cmd = QString::asprintf("GSetTCPByName,%d,", robotId) + name + ",;";
    return CmdContain(cmd);
}

CmdContain HansCommand::SetUCSByName(int robotId, QString name) {
    QString cmd = QString::asprintf("SetUCSByName,%d,", robotId) + name + ",;";
    return CmdContain(cmd);
}

CmdContain HansCommand::SetOverride(int robotId, int override) {
    QString cmd = QString::asprintf("SetOverride,%d,%.3f,;", robotId, override / 100.0);
    return CmdContain(cmd);
}

CmdContain HansCommand::SetEndDO(int robotId, int index, bool state) {
    QString cmd = QString::asprintf("SetEndDO,%d,%d,ioState,;", robotId, index,
                                    ((state) ? 1 : 0));
    return CmdContain(cmd);
}

CmdContain HansCommand::SetBoxDO(int index, bool state) {
    QString cmd = QString::asprintf("SetBoxDO,%d,%d,;", index, ((state) ? 1 : 0));
    return CmdContain(cmd);
}

CmdContain HansCommand::SetBoxCO(int index, bool state) {
    QString cmd = QString::asprintf("SetBoxCO,%d,%d,;", index, ((state) ? 1 : 0));
    return CmdContain(cmd);
}

CmdContain HansCommand::SetBoxAO(int index, double value, AnalogPattern pattern) {
    QString cmd = QString::asprintf("SetBoxAO,%d,%.3f,%d,;", index, value, pattern);
    return CmdContain(cmd);
}

CmdContain HansCommand::SetBoxAOMode(int index, AnalogPattern pattern) {
    QString cmd = QString::asprintf("SetBoxAOMode,%d,%d,;", index, pattern);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadEI(int robotId, int index) {
    QString cmd = QString::asprintf("ReadEI,%d,%d,;", robotId, index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadEO(int robotId, int index) {
    QString cmd = QString::asprintf("ReadEO,%d,%d,;", robotId, index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadEAI(int robotId, int index) {
    QString cmd = QString::asprintf("ReadEAI,%d,%d,;", robotId, index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxDI(int index) {
    QString cmd = QString::asprintf("ReadBoxDI,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxCI(int index) {
    QString cmd = QString::asprintf("ReadBoxCI,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxCO(int index) {
    QString cmd = QString::asprintf("ReadBoxCO,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxDO(int index) {
    QString cmd = QString::asprintf("ReadBoxDO,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxAO(int index) {
    QString cmd = QString::asprintf("ReadBoxAO,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadBoxAI(int index) {
    QString cmd = QString::asprintf("ReadBoxAI,%d,;", index);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadActPos(int robotId) {
    QString cmd = QString::asprintf("ReadActPos,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadOverride(int robotId) {
    QString cmd = QString::asprintf("ReadOverride,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadRobotState(int robotId) {
    QString cmd = QString::asprintf("ReadRobotState,%d,;", robotId);
    return CmdContain(cmd);
}

CmdContain HansCommand::ReadCurFSM(int robotId) {
    QString cmd = QString::asprintf("ReadCurFSM,%d,;", robotId);
    return CmdContain(cmd);
}

//HansCommand::CmdContain HansCommand::PCS2ACS(int robotId) {}
//HansCommand::CmdContain HansCommand::WayPoint(int robotId) {}

CmdContain HansCommand::MoveJ(int robotId, JointPoint point) {
    QString cmd = QString::asprintf("MoveJ,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,;",
                                    robotId, point.J1, point.J2, point.J3, point.J4, point.J5, point.J6);
    return  CmdContain(cmd);
}

CmdContain HansCommand::MoveL(int robotId, DescartesPoint point) {
    QString cmd = QString::asprintf("MoveL,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,;",
                                    robotId, point.X, point.Y, point.Z, point.rX, point.rY, point.rZ);
    return  CmdContain(cmd);
}

}
