#ifndef PLCDEVICE_H
#define PLCDEVICE_H

#include <QString>
#include <QRegularExpression>
#include <QMap>

namespace fx {
class FxPlc
{
public:
  FxPlc();

  enum DeviceName : int {
    kPLCDevice_D,
    kPLCDevice_R,
    kPLCDevice_TN,
    kPLCDevice_TS,
    kPLCDevice_CN,
    kPLCDevice_CS,
    kPLCDevice_X,
    kPLCDevice_Y,
    kPLCDevice_M,
    kPLCDevice_S,
    kPLCDevice_Unknown
  };

  struct DeviceAddress {
    DeviceName name;
    QString name_string;
    int number;
    QString raw_address;

    DeviceAddress() {}
    DeviceAddress(DeviceName device_name, int device_number) {
      name = device_name;
      number = device_number;
      name_string = EnumToQString(device_name);
      raw_address = name_string + QString::number(device_number);
    }
    bool FromString(QString input_name);
  };

  struct DeviceMap
  {
    QMap<int, bool> m_map;
    QMap<int, bool> y_map;
    QMap<int, quint16> d_map;

    DeviceMap () {}
    void Add(DeviceName device_name, int device_number);
    void Add(DeviceName device_name, int device_number, bool value);
    void Add(DeviceName device_name, int device_number, quint16 value);
    void ClearAll();
    void Remove(DeviceName device_name, int device_number);
  };

  static DeviceName QStringToEnum(QString str);
  static QString EnumToQString(DeviceName name);
};
}
#endif // PLCDEVICE_H
