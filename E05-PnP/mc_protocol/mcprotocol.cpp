#include "mcprotocol.h"

namespace fx {
McProtocol::McProtocol() {}

QByteArray McProtocol::RemoteRun(quint16 monitor_time,
                                 quint8 pc_number) {
  QByteArray frame;
  frame.clear();
  PushBackUint8(frame, kSubHeader_RemoteRun);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  return frame;
}

QByteArray McProtocol::RemoteStop(quint16 monitor_time,
                                  quint8 pc_number) {
  QByteArray frame;
  frame.clear();
  PushBackUint8(frame, kSubHeader_RemoteStop);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  return frame;
}

QByteArray McProtocol::ReadPlcName(quint16 monitor_time,
                                   quint8 pc_number) {
  QByteArray frame;
  frame.clear();
  PushBackUint8(frame, kSubHeader_ReadPlcName);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  return frame;
}

QByteArray McProtocol::BatchReadBitUnits(FxPlc::DeviceName device_name,
                                         quint32 device_number,
                                         quint8 read_size,
                                         quint16 monitor_time,
                                         quint8 pc_number) {
  QByteArray frame;
  frame.clear();

  if ((device_name != FxPlc::kPLCDevice_X) &&
      (device_name != FxPlc::kPLCDevice_Y) &&
      (device_name != FxPlc::kPLCDevice_M) &&
      (device_name != FxPlc::kPLCDevice_S) &&
      (device_name != FxPlc::kPLCDevice_TS) &&
      (device_name != FxPlc::kPLCDevice_CS)) {
    return frame;
  }

  PushBackUint8(frame, kSubHeader_ReadBit);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  AddHeadDevice(frame, device_number, device_name);
  PushBackUint8(frame, read_size);
  PushBackUint8(frame, 0x00);
  return frame;
}

QByteArray McProtocol::BatchRWriteBitUnits(FxPlc::DeviceName device_name,
                                           quint32 device_number,
                                           QList<bool> &value,
                                           quint16 monitor_time,
                                           quint8 pc_number) {
  QByteArray frame;
  frame.clear();

  if ((device_name != FxPlc::kPLCDevice_X) &&
      (device_name != FxPlc::kPLCDevice_Y) &&
      (device_name != FxPlc::kPLCDevice_M) &&
      (device_name != FxPlc::kPLCDevice_S) &&
      (device_name != FxPlc::kPLCDevice_TS) &&
      (device_name != FxPlc::kPLCDevice_CS)) {
    return frame;
  }

  PushBackUint8(frame, kSubHeader_WriteBit);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  AddHeadDevice(frame, device_number, device_name);
  PushBackUint8(frame, value.size());
  PushBackUint8(frame, 0x00);
  for (int index=0;index<value.size();index+=2) {
    char byte_low = value[index] ? 0x10 : 0x00;
    char byte_high = value[index+1] ? 0x01 : 0x00;
    char send_byte = byte_low + byte_high;
    frame.push_back(send_byte);
  }
  return frame;
}

QByteArray McProtocol::BatchReadWordUnits(FxPlc::DeviceName device_name,
                                     quint32 device_number,
                                     quint8 read_size,
                                     quint16 monitor_time,
                                     quint8 pc_number) {
  QByteArray frame;
  frame.clear();

  PushBackUint8(frame, kSubHeader_ReadWord);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  AddHeadDevice(frame, device_number, device_name);
  PushBackUint8(frame, read_size);
  PushBackUint8(frame, 0x00);
  return frame;
}

QByteArray McProtocol::BatchRWriteWordUnits(FxPlc::DeviceName device_name,
                                       quint32 device_number,
                                       QList<quint16> &value,
                                       quint16 monitor_time,
                                       quint8 pc_number) {
  QByteArray frame;
  frame.clear();

  PushBackUint8(frame, kSubHeader_WriteWord);
  PushBackUint8(frame, pc_number);
  PushBackUint16(frame, monitor_time);
  AddHeadDevice(frame, device_number, device_name);
  PushBackUint8(frame, value.size());
  PushBackUint8(frame, 0x00);
  for (int index=0;index<value.size();index++) {
    PushBackUint16(frame, value[index]);
  }
  return frame;
}

QString McProtocol::ConvertToPLCName(quint8 name_code) {
  if (name_code == 0xF3) return "FX3U/FX3UC";
  if (name_code == 0xF4) return "FX3G/FX3GC";
  if (name_code == 0xF5) return "FX3S";
  return "Unknown";
}

bool McProtocol::ParseResponse(QByteArray &response, McResult& result) {
  result.sub_header = static_cast<SubHeaderCode>(response[0] - 0x80);
  result.complete_code = static_cast<CompletionCode>(response[1]);

  if (result.complete_code != kComplete_Normal) {
    if (result.complete_code == kComplete_Abnormal) {
      result.abnormal_code = static_cast<AbnormalCode>(response[2]);
    }
    return false;
  }

  int point_num = response.length() - 2;
  char *data_ptr = response.data() + 2;
  switch (result.sub_header) {
    case kSubHeader_ReadBit:
      ConvertToBitUnits(data_ptr, point_num, result.response_data);
      return true;
    case kSubHeader_ReadWord:
      ConvertToWordUnits(data_ptr, point_num, result.response_data);
      return true;
    case kSubHeader_WriteBit:
      return true;
    case kSubHeader_WriteWord:
      return true;
    case kSubHeader_RemoteRun:
      return true;
    case kSubHeader_RemoteStop:
      return true;
    case kSubHeader_ReadPlcName:
      result.response_data.clear();
      result.response_data.push_back(response[2]);
      result.complete_code = static_cast<CompletionCode>(response[3]);
      return true;
  }
  result.is_sub_header_not_found = true;
  return false;
}

void McProtocol::AddHeadDevice(QByteArray &byte_array,
                               quint32& device_number,
                               FxPlc::DeviceName &device_name) {
  PushBackUint32(byte_array, device_number);
  PushBackUint16(byte_array, GetDeviceCode(device_name));
}

quint16 McProtocol::GetDeviceCode(FxPlc::DeviceName device_name) {
  switch (device_name) {
    case FxPlc::kPLCDevice_D:
      return 0x4420;
    case FxPlc::kPLCDevice_R:
      return 0x5220;
    case FxPlc::kPLCDevice_TN:
      return 0x544E;
    case FxPlc::kPLCDevice_TS:
      return 0x5453;
    case FxPlc::kPLCDevice_CN:
      return 0x434E;
    case FxPlc::kPLCDevice_CS:
      return 0x4353;
    case FxPlc::kPLCDevice_X:
      return 0x5820;
    case FxPlc::kPLCDevice_Y:
      return 0x5920;
    case FxPlc::kPLCDevice_M:
      return 0x4D20;
    case FxPlc::kPLCDevice_S:
      return 0x5320;
  }
  return 0x0000;
}

void McProtocol::PushBackUint8(QByteArray &byte_array, quint8 value) {
  byte_array.push_back(static_cast<char>(value));
}

void McProtocol::PushBackUint16(QByteArray &byte_array, quint16 value) {
  char low_byte = static_cast<char>(value);
  char high_byte = static_cast<char>(value >> 8);
  byte_array.push_back(low_byte);
  byte_array.push_back(high_byte);
}

void McProtocol::PushBackUint32(QByteArray &byte_array, quint32 value) {
  for(int index=0;index<4;index++) {
    char byte_value = static_cast<char>(value >> 8*index);
    byte_array.push_back(byte_value);
  }
}

void McProtocol::ConvertToBitUnits(char *ptr, int point_num, QList<char> &container) {
  container.clear();
  for(int index=0;index<point_num;index++) {
    char value = *ptr++;
    container.push_back((value & 0x10) >> 4);
    container.push_back((value & 0x01));
  }
}

void McProtocol::ConvertToWordUnits(char *ptr, int point_num, QList<char> &container) {
  container.clear();
  for(int index=0;index<point_num;index++) {
    char value = *ptr++;
    container.push_back(value);
  }
}
}
