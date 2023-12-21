#include "dh_pgc.h"

namespace dhr {
DH_PGC::DH_PGC() {
  is_first_update_data_ = true;
  InitDataMap();
}

DH_PGC::DH_PGC(int slave_address) {
  slave_address_ = slave_address;
  is_first_update_data_ = true;
  InitDataMap();
}

void DH_PGC::SetSlaveAddress(int address) {
  if(IsValueInRange(SLAVE_ADDRESS_MAX, SLAVE_ADDRESS_MIN, address)) {
    slave_address_ = address;
  }
}

void DH_PGC::UpdateData(const QModbusDataUnit unit) {
  for (qsizetype index=0;index<unit.valueCount();++index) {
    quint16 address = static_cast<quint16>(unit.startAddress()) +
                      static_cast<quint16>(index);
    if (data_map_.contains(address)) {
      data_map_[address] = unit.value(index);
    }
  }

  if (is_first_update_data_) {
    last_grip_state_ = data_map_[feedback_grip_status];
    is_first_update_data_ = false;
    return;
  }

  if (last_grip_state_ != data_map_[feedback_grip_status]) {
    is_gripper_state_change_ = true;
    last_grip_state_ = data_map_[feedback_grip_status];
    return;
  }
}

PGCData DH_PGC::DeviceInfo() {
  PGCData rgi_info;
  ConvertFeedbackData(rgi_info);
  return rgi_info;
}

bool DH_PGC::IsGripperStateChange(DhGripperStatus &state) {
  bool result = is_gripper_state_change_;
  if (result) { is_gripper_state_change_ = false; }
  state = static_cast<DhGripperStatus>(data_map_[feedback_grip_status]);
  return result;
}

ModbusFunc DH_PGC::GetDeviceFeedbackInfo() {
  return GetDeviceFeedbackInfo(slave_address_);
}

ModbusFunc DH_PGC::SetInitDevice() {
  return SetInitDevice(slave_address_);
}

ModbusFunc DH_PGC::SetGripperPosition(int position) {
  return SetGripperPosition(slave_address_, position);
}

ModbusFunc DH_PGC::SetGripperForce(int force) {
  return SetGripperForce(slave_address_, force);
}

ModbusFunc DH_PGC::SetGripperSpeed(int speed) {
  return SetGripperSpeed(slave_address_, speed);
}

ModbusFunc DH_PGC::GetDeviceFeedbackInfo(int slave_address) {
  ModbusFunc func(slave_address, FuncCode::kFuncReadHoldingRegs,
                  feedback_grip_init_state, static_cast<quint16>(0x18));
  return func;
}

ModbusFunc DH_PGC::SetInitDevice(int slave_address) {
  ModbusFunc func(slave_address, FuncCode::kFuncWriteHoldingRegs,
                  control_init, 1);
  func.value.push_back(165);
  return func;
}

ModbusFunc DH_PGC::SetGripperPosition(int slave_address, int position) {
  ModbusFunc func(slave_address, FuncCode::kFuncWriteHoldingRegs,
                  control_grip_position, 1);
  func.value.push_back(position);
  if(!IsValueInRange(GRIPPER_POSITION_MAX, GRIPPER_POSITION_MIN, position)) {
    func.slave_address = 255;
  }
  return func;
}

ModbusFunc DH_PGC::SetGripperForce(int slave_address, int force) {
  ModbusFunc func(slave_address, FuncCode::kFuncWriteHoldingRegs,
                  control_grip_force, 1);
  func.value.push_back(force);
  if(!IsValueInRange(GRIPPER_FORCE_MAX, GRIPPER_FORCE_MIN, force)) {
    func.slave_address = 255;
  }
  return func;
}

ModbusFunc DH_PGC::SetGripperSpeed(int slave_address, int speed) {
  ModbusFunc func(slave_address, FuncCode::kFuncWriteHoldingRegs,
                  control_grip_speed, 1);
  func.value.push_back(speed);
  if(!IsValueInRange(GRIPPER_SPEED_MAX, GRIPPER_SPEED_MIN, speed)) {
    func.slave_address = 255;
  }
  return func;
}

void DH_PGC::InitDataMap() {
  ///// MAPPING FEEDBACK DATA
  // control map
  data_map_[control_init] = 0x00;
  data_map_[control_grip_force] = 0x00;
  data_map_[control_grip_position] = 0x00;
  data_map_[control_grip_speed] = 0x00;
  // feedback map
  data_map_[feedback_grip_init_state] = 0x00;
  data_map_[feedback_grip_status] = 0x00;
  data_map_[feedback_grip_position] = 0x00;
  data_map_[feedback_grip_speed] = 0x00;
  data_map_[feedback_grip_current] = 0x00;
  data_map_[feedback_error_code] = 0x00;
}

void DH_PGC::ConvertFeedbackData(PGCData &pgc_info) {
  pgc_info.feedback.gripper_init_state =
      static_cast<DhInitState>(data_map_[feedback_grip_init_state]);
  pgc_info.feedback.gripper_status =
      static_cast<DhGripperStatus>(data_map_[feedback_grip_status]);
  pgc_info.feedback.gripper_position = data_map_[feedback_grip_position];
  pgc_info.feedback.gripper_speed = data_map_[feedback_grip_speed];
  pgc_info.feedback.gripper_current = data_map_[feedback_grip_current];
  pgc_info.feedback.error_code =
      static_cast<DhError>(data_map_[feedback_error_code]);
}
}
