#ifndef DH_PGC_H
#define DH_PGC_H

#include "dhr_define.h"

namespace dhr {
class PGCData
{
public:
  struct PGCFeedback
  {
    DhInitState gripper_init_state = DhInitState::kInitNone;
    DhGripperStatus gripper_status = DhGripperStatus::kGriperAtPosition;
    int gripper_position = 0x00;
    int gripper_speed = 0x00;
    int gripper_current = 0x00;
    DhError error_code = DhError::kErrorNone;
  };

  DhModelInfo model_info;
  PGCFeedback feedback;
};

class DH_PGC
{
public:
  DH_PGC();
  DH_PGC(int slave_address = 1);

  void SetSlaveAddress(int address);
  void UpdateData(const QModbusDataUnit unit);
  PGCData DeviceInfo();
  bool IsGripperStateChange(DhGripperStatus &state);

  ModbusFunc GetDeviceFeedbackInfo();
  ModbusFunc SetInitDevice();
  ModbusFunc SetGripperPosition(int position);
  ModbusFunc SetGripperForce(int force);
  ModbusFunc SetGripperSpeed(int speed);

  static ModbusFunc GetDeviceFeedbackInfo(int slave_address);
  static ModbusFunc SetInitDevice(int slave_address);
  static ModbusFunc SetGripperPosition(int slave_address, int position);
  static ModbusFunc SetGripperForce(int slave_address, int force);
  static ModbusFunc SetGripperSpeed(int slave_address, int speed);

private:
  void InitDataMap();
  void ConvertFeedbackData(PGCData &pgc_info);

public:
  int slave_address_;

  static const quint16 control_init = 0x0100;
  static const quint16 control_grip_force = 0x0101;
  static const quint16 control_grip_position = 0x0103;
  static const quint16 control_grip_speed = 0x0104;

  static const quint16 feedback_grip_init_state = 0x0200;
  static const quint16 feedback_grip_status = 0x0201;
  static const quint16 feedback_grip_position = 0x0202;
  static const quint16 feedback_grip_speed = 0x0203;
  static const quint16 feedback_grip_current = 0x0204;
  static const quint16 feedback_error_code = 0x0205;

private:
  QMap<quint16, quint16> data_map_;
  bool is_first_update_data_ = true;
  quint16 last_grip_state_;
  bool is_gripper_state_change_;
};
}

#endif // DH_PGC_H
