#include "ModbusExpandIO.h"
#include "qvariant.h"

ModbusExpandIO::ModbusExpandIO(QObject *parent)
    : QObject{parent} {
  modbus_device_ = new QModbusTcpClient(parent);
  is_connected_ = false;

  connect(modbus_device_, &QModbusClient::stateChanged, this, &ModbusExpandIO::DeviceChangedState);
}

ModbusExpandIO::~ModbusExpandIO() {
  if (modbus_device_) {
    modbus_device_->disconnectDevice();
  }
}

void ModbusExpandIO::ModuleIoConnect(QString host, int port) {
  if(modbus_device_->state() != QModbusDevice::UnconnectedState) {
    return;
  }

  if(host.isEmpty()) {
    return;
  }

  server_host_ = host;
  server_port_ = port;

  // set modbus host address and port
  modbus_device_->setConnectionParameter(QModbusDevice::NetworkAddressParameter,
                                         server_host_);
  modbus_device_->setConnectionParameter(QModbusDevice::NetworkPortParameter,
                                         server_port_);

  // set modbus connect timeout and number of retries time
  modbus_device_->setTimeout(MODBUS_RESPONSE_TIME);
  modbus_device_->setNumberOfRetries(MODBUS_NUM_OF_RETRIES);

  // connect, if connect fail emit corresponding signal
  if (!modbus_device_->connectDevice()) {
    emit ModuleIoConnectInitFail(
        tr("Connect failed: %1").arg(modbus_device_->errorString()));
  }
}

void ModbusExpandIO::ModuleIoDisconnect() {
  modbus_device_->disconnectDevice();
}

bool ModbusExpandIO::IsModuleIoConnected() {
  return is_connected_;
}

void ModbusExpandIO::WriteCoils(QModbusDataUnit writeUnit) {
  if(!is_connected_) {
    return;
  }

  QModbusReply *reply = modbus_device_->sendWriteRequest(writeUnit, server_port_);
  if(reply) {
    if(!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, [this, reply]() {
        const auto error = reply->error();
        if (error == QModbusDevice::ProtocolError) {
          emit ModuleIoWriteResponseError(
              tr("Write response error: %1 (Modbus exception: 0x%2)")
                  .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        } else if (error != QModbusDevice::NoError) {
          emit ModuleIoWriteResponseError(
              tr("Write response error: %1 (code: 0x%2)")
                  .arg(reply->errorString()).arg(error, -1, 16));
        } else if (error == QModbusDevice::NoError) {
          RefreshIoData(reply->result());
          emit ModuleIoDataChanged(io_);
        }
        reply->deleteLater();
      });
    }
    else {
      reply->deleteLater();
    }
  }
  else {
    emit ModuleIoWriteError(tr("Write error: %1").arg(modbus_device_->errorString()));
  }
}

void ModbusExpandIO::WriteCoils(int address, bool state) {
  QModbusDataUnit writeUnit(QModbusDataUnit::Coils, address, 1);
  writeUnit.setValue(0, state);
  WriteCoils(writeUnit);
}

void ModbusExpandIO::ReadCoils() {

}

void ModbusExpandIO::RefreshIoData(QModbusDataUnit unit) {
  if(unit.valueCount() < 0) {
    return;
  }

  switch (unit.registerType()) {
    case QModbusDataUnit::Coils:
      UpdateOuptut(unit);
      break;
    case QModbusDataUnit::InputRegisters:
      UpdateInput(unit);
      break;
  }
}

void ModbusExpandIO::UpdateInput(QModbusDataUnit &unit) {
  for (qsizetype index = 0, total = unit.valueCount(); index < total; ++index) {
    io_.SetInputAt(unit.startAddress() + index, unit.value(index));
  }
}

void ModbusExpandIO::UpdateOuptut(QModbusDataUnit &unit) {
  for (qsizetype index = 0, total = unit.valueCount(); index < total; ++index) {
    io_.SetOutputAt(unit.startAddress() + index, unit.value(index));
  }
}

void ModbusExpandIO::DeviceChangedState(QModbusDevice::State state) {
  switch (state) {
    case QModbusDevice::ConnectedState:
      is_connected_ = true;
      emit ModuleIoConnected();
//      readFeederInitValue();
      break;
    case QModbusDevice::UnconnectedState:
      if(!is_connected_) {
        emit ModuleIoConnectFail();
        return;
      }
      is_connected_ = false;
      emit ModuleIoDisconnected();
      break;
    case QModbusDevice::ConnectingState:
      emit ModuleIoConnecting();
      break;
  }
}
