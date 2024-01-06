#include "plcdevice.h"

namespace fx {
FxPlc::FxPlc() {}

FxPlc::DeviceName FxPlc::QStringToEnum(QString str) {
  if (str == "D") return kPLCDevice_D;
  if (str == "R") return kPLCDevice_R;
  if (str == "TN") return kPLCDevice_TN;
  if (str == "TS") return kPLCDevice_TS;
  if (str == "CN") return kPLCDevice_CN;
  if (str == "CS") return kPLCDevice_CS;
  if (str == "X") return kPLCDevice_X;
  if (str == "Y") return kPLCDevice_Y;
  if (str == "M") return kPLCDevice_M;
  if (str == "S") return kPLCDevice_S;
  return kPLCDevice_Unknown;
}

QString FxPlc::EnumToQString(DeviceName name) {
  if (name == kPLCDevice_D) return "D";
  if (name == kPLCDevice_R) return "R";
  if (name == kPLCDevice_TN) return "TN";
  if (name == kPLCDevice_TS) return "TS";
  if (name == kPLCDevice_CN) return "CN";
  if (name == kPLCDevice_CS) return "CS";
  if (name == kPLCDevice_X) return "X";
  if (name == kPLCDevice_Y) return "Y";
  if (name == kPLCDevice_M) return "M";
  if (name == kPLCDevice_S) return "S";
  return "unknown";
}

bool FxPlc::DeviceAddress::FromString(QString input_name) {
  QString expression = "^(D|R|TN|TS|CN|CS|X|Y|M|S)(\\d+)";
  QRegularExpressionMatch *match = new QRegularExpressionMatch;
  raw_address = input_name;
  if (input_name.contains(QRegularExpression(expression), match)) {
    name = QStringToEnum(match->captured(1));
    name_string = match->captured(1);
    number = match->captured(2).toInt();
    return true;
  }
  return false;
}

void FxPlc::DeviceMap::Add(DeviceName device_name, int device_number) {
  switch (device_name) {
    case FxPlc::kPLCDevice_M:
      m_map[device_number] = false;
      break;
    case FxPlc::kPLCDevice_Y:
      y_map[device_number] = false;
      break;
    case FxPlc::kPLCDevice_D:
      d_map[device_number] = 0;
      break;
  }
}

void FxPlc::DeviceMap::Add(DeviceName device_name,
                               int device_number,
                               bool value) {
  switch (device_name) {
    case FxPlc::kPLCDevice_M:
      m_map[device_number] = value;
      break;
    case FxPlc::kPLCDevice_Y:
      y_map[device_number] = value;
      break;
  }
}

void FxPlc::DeviceMap::Add(DeviceName device_name,
                               int device_number,
                               quint16 value) {
  switch (device_name) {
    case FxPlc::kPLCDevice_D:
      d_map[device_number] = value;
      break;
  }
}

void FxPlc::DeviceMap::ClearAll() {
  m_map.clear();
  y_map.clear();
  d_map.clear();
}

void FxPlc::DeviceMap::Remove(DeviceName device_name, int device_number) {
  switch (device_name) {
    case FxPlc::kPLCDevice_M:
      m_map.remove(device_number);
      break;
    case FxPlc::kPLCDevice_Y:
      y_map.remove(device_number);
      break;
    case FxPlc::kPLCDevice_D:
      d_map.remove(device_number);
      break;
  }
}
}
