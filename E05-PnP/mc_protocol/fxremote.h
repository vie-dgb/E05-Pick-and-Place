#ifndef FXREMOTE_H
#define FXREMOTE_H

#include <QThread>
#include <QObject>
#include <QTcpSocket>
#include <QList>
#include <QMutex>
#include <QDebug>

#include "mcprotocol.h"
#include "plcdevice.h"
#include "time/TimeCounter.h"

#define FX_CONNECT_TIMEOUT    2000
#define FX_MUTEX_BLOCK_TIME   50

namespace fx {
class FxRemote : public QThread
{
  Q_OBJECT
public:
  struct SendCommand {
    FxPlc::DeviceName device_name;
    quint32 device_start_number;
    QByteArray send_string;
  };

  struct DeviceRange {
    int start;
    int number;
  };

  FxRemote(QThread::Priority priority = QThread::NormalPriority);
  ~FxRemote();

  void FxSetConnectTimeOut(int time_out);
  void FxSetReponseTimeOut(int time_out);
  void FxConnectToPlc(QString address, int port);
  void FxDisconnectFromPlc();
  bool FxIsConnected();

  void FxWriteBit(FxPlc::DeviceName device_name,
                  quint32 device_number,
                  bool value);
  void FxWriteWord(FxPlc::DeviceName device_name,
                   quint32 device_number,
                   quint16 value);
  void FxSetPlcRunMode();
  void FxSetPlcStopMode();
  void FxGetModelName();
  QString FxPlcModelName();
  bool GetAuxiliaryRelayState(int device_number, bool &get_value);
  bool FxIsPlcRunMode();
  bool FxIsPlcErrorOccurred();
  void FxSetDeviceMap(FxPlc::DeviceMap &device_map);

private:
  void run() override;
  void InitSocket();
  void DeleteSocket();
  void PollingQueryHandle();
  void QueryDeviceM();
  void QueryDeviceY();
  void QueryDeviceD();
  void QuerySpecialRelay();
  void QueueCommandHandle();
  void SendRequest(SendCommand  &command);
  void ResponseHandle(McProtocol::McResult &result);
  void SpecialAuxiliaryRelayCheck(int &relay_number, char &value);

  void FindQueryList(QList<int> &raw_list,
                     QList<DeviceRange> &query_list);

signals:
  void FxSignal_Connected();
  void FxSignal_Disconnected();
  void FxSignal_ConnectFail();
  void FxSignal_PollingQueryStriggered(FxPlc::DeviceMap device_map);
  void FxSingal_ModifyDeviceBit(FxPlc::DeviceName device_name, bool value);
  void FxSingal_ModifyDeviceWord(FxPlc::DeviceName device_name, quint16 value);
  void FxSignal_DeivceMChangedState(int m_number, bool value);
  void FxSignal_DeivceYChangedState(int m_number, bool value);
  void FxSignal_DeviceDChangedValue(int d_number, int value);

private:
  QThread::Priority thread_priority_;
  bool is_thread_running_;

  QTcpSocket *socket_;
  QString connect_address_;
  int connect_port_;

  bool is_fx_connected = false;
  int connect_time_out_ = FX_CONNECT_TIMEOUT;

  quint16 monitoring_time_ = 0x0A;
  quint8 pc_number_ = 0xFF;
  int response_time_out_ = 3000;

  QMutex mutex_data_;
  QList<SendCommand> send_queue_;
  FxPlc::DeviceMap device_map_;
  QList<DeviceRange> query_list_y_;
  QList<DeviceRange> query_list_m_;
  QList<DeviceRange> query_list_d_;
  QString plc_model_name_;
  bool plc_is_running_mode_;
  bool plc_is_error_occured_;

  TimeCounter *polling_time_counter_;
  const int polling_time_out_ = 100;
};
}
#endif // FXREMOTE_H
