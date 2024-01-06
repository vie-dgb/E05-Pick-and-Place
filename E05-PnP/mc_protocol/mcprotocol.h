#ifndef MCPROTOCOL_H
#define MCPROTOCOL_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <stdint.h>
#include "plcdevice.h"

namespace fx {
class McProtocol
{
public:
  enum ResponseCode : int {
    kResponse_Ok,
    kResponse_NotFoundHeader,
  };

  enum SubHeaderCode : char {
    kSubHeader_ReadBit = 0x00,
    kSubHeader_ReadWord = 0x01,
    kSubHeader_WriteBit = 0x02,
    kSubHeader_WriteWord = 0x03,
    kSubHeader_TestBit = 0x04,
    kSubHeader_TestWord = 0x05,
    kSubHeader_RemoteRun = 0x13,
    kSubHeader_RemoteStop = 0x14,
    kSubHeader_ReadPlcName = 0x15
  };

  enum CompletionCode : char {
    kComplete_Normal = 0x00,
    kComplete_CommandCodeWrong = 0x50,
    kComplete_SendFrameWrong = 0x54,
    kComplete_DeviceNameWrong = 0x56,
    kComplete_PointsNumWrong = 0x57,
    kComplete_HeadDeviceWrong = 0x58,
    kComplete_Abnormal = 0x5B,
    kComplete_MonitorTimeOut = 0x60
  };

  enum AbnormalCode : char {
    kAbnormal_PcNumErr = 0x10,
    kAbnormal_ModeErr = 0x11,
    kAbnormal_Unknown = 0x12,
    kAbnormal_RemoteErr = 0x18
  };

  struct McResult
  {
    McResult() {}

    FxPlc::DeviceName device_name = FxPlc::kPLCDevice_Unknown;
    int device_start_number = 0;
    SubHeaderCode sub_header = kSubHeader_ReadBit;
    bool is_sub_header_not_found = false;
    CompletionCode complete_code = kComplete_Normal;
    AbnormalCode abnormal_code = kAbnormal_Unknown;
    QList<char> response_data;
  };

  McProtocol();

  static QByteArray RemoteRun(quint16 monitor_time = 0x0A, quint8 pc_number = 0xFF);
  static QByteArray RemoteStop(quint16 monitor_time = 0x0A, quint8 pc_number = 0xFF);
  static QByteArray ReadPlcName(quint16 monitor_time = 0x0A, quint8 pc_number = 0xFF);
  static QByteArray BatchReadBitUnits(FxPlc::DeviceName device_name,
                                      quint32 device_number, quint8 read_size,
                                      quint16 monitor_time, quint8 pc_number = 0xFF);
  static QByteArray BatchRWriteBitUnits(FxPlc::DeviceName device_name,
                                        quint32 device_number, QList<bool> &value,
                                        quint16 monitor_time, quint8 pc_number = 0xFF);
  static QByteArray BatchReadWordUnits(FxPlc::DeviceName device_name,
                                       quint32 device_number, quint8 read_size,
                                       quint16 monitor_time, quint8 pc_number = 0xFF);
  static QByteArray BatchRWriteWordUnits(FxPlc::DeviceName device_name,
                                         quint32 device_number, QList<quint16> &value,
                                         quint16 monitor_time, quint8 pc_number = 0xFF);
  static QString ConvertToPLCName(quint8 name_code);
  static bool ParseResponse(QByteArray &response, McResult& result);

private:
  static void AddHeadDevice(QByteArray &byte_array,
                            quint32& device_number, FxPlc::DeviceName &device_name);
  static quint16 GetDeviceCode(FxPlc::DeviceName device_name);
  static void PushBackUint8(QByteArray &byte_array, quint8 value);
  static void PushBackUint16(QByteArray &byte_array, quint16 value);
  static void PushBackUint32(QByteArray &byte_array, quint32 value);
  static void ConvertToBitUnits(char *ptr, int point_num, QList<char> &container);
  static void ConvertToWordUnits(char *ptr, int point_num, QList<char> &container);
};
}
#endif // MCPROTOCOL_H
