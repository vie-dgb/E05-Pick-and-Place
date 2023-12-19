#ifndef DH_RGI_H
#define DH_RGI_H

#include <QtGlobal>
#include <QMap>
#include <QModbusDataUnit>
#include <QDebug>
#include "dhr_define.h"

#define GRIPPER_POSITION_MAX    1000
#define GRIPPER_POSITION_MIN    0
#define GRIPPER_FORCE_MAX       100
#define GRIPPER_FORCE_MIN       20
#define GRIPPER_SPEED_MAX       100
#define GRIPPER_SPEED_MIN       0

#define ROTATION_ANGLE_MAX      1800
#define ROTATION_ANGLE_MIN      -1800
#define ROTATION_TORQUE_MAX     100
#define ROTATION_TORQUE_MIN     20
#define ROTATION_SPEED_MAX      100
#define ROTATION_SPEED_MIN      0

namespace dhr {

class RGIData
{
public:
  struct RGIModel
  {
    quint16 model = 0x00;
    quint16 hw_version = 0x00;
    quint16 sw_version = 0x00;
    quint16 unique_id_low = 0x00;
    quint16 unique_id_high = 0x00;
  };

  struct RGIFeedback
  {
    DhInitState gripper_init_state = DhInitState::kInitNone;
    DhGripperStatus gripper_status = DhGripperStatus::kGriperAtPosition;
    int gripper_position = 0x00;
    int gripper_speed = 0x00;
    int gripper_current = 0x00;
    DhInitState rotation_init_state = DhInitState::kInitNone;
    DhRotationStatus rotation_status = DhRotationStatus::kRotationAtPosition;
    int rotation_speed = 0x00;
    int rotation_current = 0x00;
    int rotation_angle = 0x00;
    DhError error_code = DhError::kErrorNone;
  };

  RGIModel device;
  RGIFeedback feedback;
};

class DH_RGI
{
public:
  DH_RGI();
  DH_RGI(int slave_address = 1);

  void SetSlaveAddress(int address);
  void UpdateData(const QModbusDataUnit unit);
  RGIData DeviceInfo();

  ModbusFunc GetDeviceFeedbackInfo();
  ModbusFunc SetInitDevice();
  ModbusFunc SetGripperPosition(int position);
  ModbusFunc SetGripperForce(int force);
  ModbusFunc SetGripperSpeed(int speed);
  ModbusFunc SetRotationAngle(int angle);
  ModbusFunc SetRotationTorque(int torque);
  ModbusFunc SetRotationSpeed(int speed);

  static ModbusFunc GetDeviceFeedbackInfo(int slave_address);
  static ModbusFunc SetInitDevice(int slave_address);
  static ModbusFunc SetGripperPosition(int slave_address, int position);
  static ModbusFunc SetGripperForce(int slave_address, int force);
  static ModbusFunc SetGripperSpeed(int slave_address, int speed);
  static ModbusFunc SetRotationAngle(int slave_address, int angle);
  static ModbusFunc SetRotationTorque(int slave_address, int torque);
  static ModbusFunc SetRotationSpeed(int slave_address, int speed);

private:
  void InitDataMap();
  void ConvertFeedbackData(RGIData &rgi_info);

public:
  int slave_address_;

  static const quint16 control_init = 0x0100;
  static const quint16 control_grip_force = 0x0101;
  static const quint16 control_grip_position = 0x0103;
  static const quint16 control_grip_speed = 0x0104;
  static const quint16 control_rotation_angle = 0x0105;
  static const quint16 control_rotation_speed = 0x0107;
  static const quint16 control_rotation_torque = 0x0108;

  static const quint16 feedback_grip_init_state = 0x0200;
  static const quint16 feedback_grip_status = 0x0201;
  static const quint16 feedback_grip_position = 0x0202;
  static const quint16 feedback_grip_speed = 0x0203;
  static const quint16 feedback_grip_current = 0x0204;
  static const quint16 feedback_error_code = 0x0205;
  static const quint16 feedback_rotation_angle = 0x0208;
  static const quint16 feedback_rotation_init_state = 0x020A;
  static const quint16 feedback_rotation_status = 0x020B;
  static const quint16 feedback_rotation_speed = 0x020C;
  static const quint16 feedback_rotation_current = 0x020D;

private:
  QMap<quint16, quint16> data_map_;
  RGIData device_info_;
};
}


#endif // DH_RGI_H
