#ifndef DHR_DEFINE_H
#define DHR_DEFINE_H

#include <QList>
#include <QString>
#include <QtGlobal>

namespace dhr {

#define SLAVE_ADDRESS_MIN   1
#define SLAVE_ADDRESS_MAX   247

enum FuncCode : int {
  kFuncReadHoldingRegs = 3,
  kFuncWriteHoldingRegs = 10
};

static QString EnumConvert(FuncCode enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case FuncCode::kFuncReadHoldingRegs:
      state_string = "Read hodling registers";
      break;
    case FuncCode::kFuncWriteHoldingRegs:
      state_string = "Write hodling registers";
      break;
  }
  return state_string;
}

enum ModbusUnitType : int {
  kUnitCoils = 0,
  kUnitInputCoils,
  kUnitHoldingRegisters,
  kUnitInputRegisters
};

static QString EnumConvert(ModbusUnitType enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case ModbusUnitType::kUnitCoils:
      state_string = "Coils";
      break;
    case ModbusUnitType::kUnitInputCoils:
      state_string = "Input coils";
      break;
    case ModbusUnitType::kUnitHoldingRegisters:
      state_string = "Holding registers";
      break;
    case ModbusUnitType::kUnitInputRegisters:
      state_string = "Input registers";
      break;
  }
  return state_string;
}

enum DhInitState : int {
  kInitNone = 0,
  kInitSuccess = 1,
  kInitInProgress = 2
};

static QString EnumConvert(DhInitState enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case DhInitState::kInitNone:
      state_string = "Not init";
      break;
    case DhInitState::kInitSuccess:
      state_string = "Init completed";
      break;
    case DhInitState::kInitInProgress:
      state_string = "Init in progress";
      break;
  }
  return state_string;
}

enum DhGripperStatus : int {
  kGriperInMotion = 0,
  kGriperAtPosition = 1,
  kGriperClampingObject = 2,
  kGriperClampingFail = 3
};

static QString EnumConvert(DhGripperStatus enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case DhGripperStatus::kGriperInMotion:
      state_string = "In motion";
      break;
    case DhGripperStatus::kGriperAtPosition:
      state_string = "Arrived position";
      break;
    case DhGripperStatus::kGriperClampingObject:
      state_string = "Clamping object";
      break;
    case DhGripperStatus::kGriperClampingFail:
      state_string = "Object falling";
      break;
  }
  return state_string;
}

enum DhRotationStatus : int {
  kRotationInMotion = 0,
  kRotationAtPosition = 1,
  kRotationBlocked = 2,
  kRotationStall = 3
};

static QString EnumConvert(DhRotationStatus enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case DhRotationStatus::kRotationInMotion:
      state_string = "In motion";
      break;
    case DhRotationStatus::kRotationAtPosition:
      state_string = "Reached position";
      break;
    case DhRotationStatus::kRotationBlocked:
      state_string = "Blocked rotation";
      break;
    case DhRotationStatus::kRotationStall:
      state_string = "Blocking stall";
      break;
  }
  return state_string;
}

enum DhError : int {
  kErrorNone = 0,
  kErrorOverheat = 4,
  kErrorOverLoad = 8,
  kErrorOverSpeed = 11
};

static QString EnumConvert(DhError enum_code) {
  QString state_string = "enum code invalid: " + enum_code;
  switch (enum_code) {
    case DhError::kErrorNone:
      state_string = "None";
      break;
    case DhError::kErrorOverheat:
      state_string = "Over heat";
      break;
    case DhError::kErrorOverLoad:
      state_string = "Over load";
      break;
    case DhError::kErrorOverSpeed:
      state_string = "Over speed";
      break;
  }
  return state_string;
}

struct ModbusFunc {
  int slave_address = 1;
  FuncCode func_code = FuncCode::kFuncReadHoldingRegs;
  quint16 start_address = 0;
  quint16 amount = 1;
  QList<quint16> value;

  ModbusFunc() {
    value.clear();
  }

  ModbusFunc(int slaveAddress, FuncCode funcCode,
             quint16 startAdress, quint16 unitAmount) {
    slave_address = slaveAddress;
    func_code = funcCode;
    start_address = startAdress;
    amount = unitAmount;
    value.clear();
  }
};

struct ModbusRegister {
  int address = 0;
  quint16 value = 0;
};

struct ModbusFuncResponse {
  int slave_address = 1;
  ModbusUnitType type = ModbusUnitType::kUnitHoldingRegisters;
  QList<ModbusRegister> value;
};

static bool IsValueInRange(int max_value, int min_value, int value) {
  return ((min_value <= value) && (value <= max_value)) ? true : false;
}
}

#endif // DHR_DEFINE_H
