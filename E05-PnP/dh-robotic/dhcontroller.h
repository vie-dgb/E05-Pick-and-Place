#ifndef DHCONTROLLER_H
#define DHCONTROLLER_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QModbusRtuSerialClient>
#include <QMutex>

#include "time/TimeCounter.h"
#include "serialsettingdialog.h"
#include "dhr_define.h"
#include "dh_rgi.h"

#define RTU_NORMAL_REFRESH_TIME           200
#define RTU_NORMAL_REFRESH_DISPLAY_TIME   500
#define RTU_CONNECT_RETRIES_TIME          3
#define RTU_NORMAL_SLAVE_ADDRESS          1
#define MUTEX_LOCK_TIMEOUT                50

namespace dhr {

class DHController : public QThread
{
  Q_OBJECT
public:
  explicit DHController(QThread::Priority priority = QThread::NormalPriority,
                        QObject *parent = nullptr);
  ~DHController();

  void DH_Connect(SerialSetting setting);
  void DH_Disconnect();
  bool DH_IsConnected();
  void DH_AddFuncToQueue(ModbusFunc func);
  void DH_SetRgiAddress(int address);

private:
  void run() override;
  void ModbusInit();
  void ModbusChangedState(QModbusDevice::State state);
  void ModbusErrorHandle(QModbusDevice::Error error);
  void ModbusReadHodlingRegister(int slave_address, int start_address, int amount);
  void ModbusWriteHoldingRegister(int slave_address, int start_address,
                                  QList<quint16> write_list);
  void ModbusHodlingRegsResponse(const int slave_address,
                                 const QModbusDataUnit unit);
  void ModbusQueueClear();
  bool ModbusQueueIsEmpty();
  ModbusFunc ModbusQueueGetFront();
  void ModbusQueuePopFront();
  void ModbusQueueHandle();
  void ModbusSendFunction(ModbusFunc func_code);

  void RGI_Init();
  void RGI_Collect_info();
  void RGI_Uinit();

signals:
  void DHSignal_Connected();
  void DHSignal_Disconnected();
  void DHSignal_ConnectFail(QString msg);
  void DHSignal_Connecting();
  void DHSignal_ErrorOccured(QString msg);
  void DHSignal_PollingTriggered(RGIData last_data);

private:
  QThread::Priority thread_priority_;
  bool thread_running_;
  bool disconnect_wait_;
  QMutex mutex_;
  QModbusRtuSerialClient *modbus_device_;
  SerialSetting setting_connect_;
  TimeCounter *refresh_time_counter_;
  TimeCounter *display_time_counter_;
  bool is_modbus_connected_;
  int refresh_data_time_;
  int refresh_display_time_;
  QList<ModbusFunc> send_func_queue_;
  DH_RGI *device_rgi;
  int device_gri_address_;
};
}

#endif // DHCONTROLLER_H
