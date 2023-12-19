#ifndef MODBUSEXPANDIO_H
#define MODBUSEXPANDIO_H

#include <QObject>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QUrl>
#include <QTimer>

#define MODBUS_RESPONSE_TIME        1000
#define MODBUS_NUM_OF_RETRIES       3

class ExpandIo
{
public:
  ExpandIo() {}

  ExpandIo(int size_input, int size_output) {
    input.resize(size_input, false);
    output.resize(size_output, false);
  }

  void SetInputSize(int num) {
    input.resize(num, false);
  }

  void SetOutputSize(int num) {
    output.resize(num, false);
  }

  void SetInputAt(int index, bool state) {
    input.at(index) = state;
  }

  bool GetInputAt(int index) {
    return input.at(index);
  }

  void SetOutputAt(int index, bool state) {
    output.at(index) = state;
  }

  bool GetOutputAt(int index) {
    return output.at(index);
  }

private:
  std::vector<bool> input;
  std::vector<bool> output;
};

class ModbusExpandIO : public QObject
{
  Q_OBJECT
public:
  explicit ModbusExpandIO(QObject *parent = nullptr);
  ~ModbusExpandIO();

  void ModuleIoConnect(QString host, int port);
  void ModuleIoDisconnect();
  bool IsModuleIoConnected();

private:
  void DeviceChangedState(QModbusDevice::State state);
  void DeviceReadReady();

  void WriteCoils(int address, bool state);
  void WriteCoils(QModbusDataUnit writeUnit);
  void ReadCoils();
  void ReadDiscreteInput();

  void RefreshIoData(QModbusDataUnit unit);
  void UpdateInput(QModbusDataUnit &unit);
  void UpdateOuptut(QModbusDataUnit &unit);

signals:
  void ModuleIoConnected();
  void ModuleIoDisconnected();
  void ModuleIoConnecting();
  void ModuleIoConnectFail();
  void ModuleIoConnectInitFail(QString msg);
  void ModuleIoWriteError(QString msg);
  void ModuleIoWriteResponseError(QString msg);
  void ModuleIoReadError(QString msg);
  void ModuleIoReadResponseError(QString msg);
  void ModuleIoDataChanged(ExpandIo io);

private:
  QString server_host_;
  int server_port_;
  QModbusClient *modbus_device_;
  bool is_connected_;
  ExpandIo io_;
};

#endif // MODBUSEXPANDIO_H
