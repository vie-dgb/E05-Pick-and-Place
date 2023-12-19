#include "dhcontroller.h"

namespace dhr {
DHController::DHController(QThread::Priority priority, QObject *parent)
    : QThread{parent} {
  thread_priority_ = priority;
  thread_running_ = false;
  is_modbus_connected_ = false;
  disconnect_wait_ = false;
  refresh_data_time_ = RTU_NORMAL_REFRESH_TIME;
  refresh_display_time_ = RTU_NORMAL_REFRESH_DISPLAY_TIME;
}

DHController::~DHController() {
  if(is_modbus_connected_) {
    DH_Disconnect();
  }
}

void DHController::DH_Connect(SerialSetting setting) {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT); 
  if(!thread_running_) {
    setting_connect_ = setting;
    thread_running_ = true;
    start(thread_priority_);
  }
  mutex_.unlock();
}

void DHController::DH_Disconnect() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  if (thread_running_) {
    disconnect_wait_ = true;
  }
  mutex_.unlock();
}

bool DHController::DH_IsConnected() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  bool is_connected = is_modbus_connected_;
  mutex_.unlock();
  return is_connected;
}

void DHController::DH_AddFuncToQueue(ModbusFunc func) {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  send_func_queue_.push_back(func);
  mutex_.unlock();
}

void DHController::DH_SetRgiAddress(int address) {
  device_gri_address_ = address;
}

void DHController::run() {
  ModbusInit();
  // for sure serial device is connected
  msleep(50);
  if(modbus_device_->state() != QModbusDevice::ConnectedState) {
    return;
  }
  // init objects and emit connected signals;
  ModbusQueueClear();
  RGI_Init();
  refresh_time_counter_ = new TimeCounter;
  display_time_counter_ = new TimeCounter;
  is_modbus_connected_ = true;
  emit DHSignal_Connected();

  while (thread_running_) {
    // disconnect flag
    if(disconnect_wait_) {
      thread_running_ = false;
      disconnect_wait_ = false;
      break;
    }

    // polling data read
    if (refresh_time_counter_->StartTimeCounter(refresh_data_time_)) {
      RGI_Collect_info();
    }
    // polling display data
    if (display_time_counter_->StartTimeCounter(refresh_display_time_)) {
      emit DHSignal_PollingTriggered(device_rgi->DeviceInfo());
    }

    ModbusQueueHandle();
  }

  RGI_Uinit();
}

void DHController::ModbusInit() {
  modbus_device_ = new QModbusRtuSerialClient;

  connect(this, &DHController::finished,
          modbus_device_, &QModbusRtuSerialClient::deleteLater);
  connect(modbus_device_, &QModbusRtuSerialClient::stateChanged,
          this, &DHController::ModbusChangedState);
  connect(modbus_device_, &QModbusRtuSerialClient::errorOccurred,
          this, &DHController::ModbusErrorHandle);

  // set serial port parameters
  modbus_device_->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                         setting_connect_.name);
  modbus_device_->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                         setting_connect_.parity);
  modbus_device_->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                         setting_connect_.baudrate);
  modbus_device_->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                         setting_connect_.data_bits);
  modbus_device_->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                         setting_connect_.stop_bits);
  modbus_device_->setTimeout(setting_connect_.response_timeout);
  modbus_device_->setNumberOfRetries(RTU_CONNECT_RETRIES_TIME);
  refresh_data_time_ = setting_connect_.polls_time;

  if(!modbus_device_->connectDevice()) {
    emit DHSignal_ConnectFail(tr("Connect failed: %1").
                                   arg(modbus_device_->errorString()));
    return;
  }
}

void DHController::ModbusChangedState(QModbusDevice::State state) {
  switch (state) {
//    case QModbusDevice::State::ConnectedState:
//      is_modbus_connected_ = true;
//      emit DHSignal_Connected();
//      break;
    case QModbusDevice::State::UnconnectedState:
      is_modbus_connected_ = false;
      emit DHSignal_Disconnected();
      break;
    case QModbusDevice::State::ConnectingState:
      emit DHSignal_Connecting();
      break;
  }
}

void DHController::ModbusErrorHandle(QModbusDevice::Error error) {
  QString message = "";
  switch (error) {
    case QModbusDevice::ReadError:
//      message = tr("Read error: ");
      break;
    case QModbusDevice::WriteError:
//      message = tr("Write error: ");
      break;
    case QModbusDevice::ConnectionError:
      message = tr("Connection error: ");
      break;
    case QModbusDevice::TimeoutError:
      message = tr("Timeout error: ");
      break;
    case QModbusDevice::ProtocolError:
//      message = tr("Protocol error: ");
      break;
    case QModbusDevice::UnknownError:
      message = tr("Unknown error");
      break;
    case QModbusDevice::InvalidResponseError:
      message = tr("Invalid response error");
      break;
  }
  if(!message.isEmpty()) {
    disconnect_wait_ = true;
    emit DHSignal_ErrorOccured(message);
  }
}

void DHController::ModbusReadHodlingRegister(int slave_address,
                                             int start_address,
                                             int amount) {
  if(!is_modbus_connected_) {
    return;
  }

  QModbusDataUnit read_request(QModbusDataUnit::HoldingRegisters,
                               start_address,
                               static_cast<quint16>(amount));
  if (auto *reply = modbus_device_->sendReadRequest(read_request, slave_address)) {
    if (!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, [this, reply] () {
        const auto error = reply->error();
        if (error == QModbusDevice::ProtocolError) {
          emit DHSignal_ErrorOccured(tr("Read error: %1 (Modbus exception: 0x%2)").
                                     arg(reply->errorString()).
                                     arg(reply->rawResult().exceptionCode(), -1, 16));
        } else if (error != QModbusDevice::NoError) {
          emit DHSignal_ErrorOccured(tr("Read error: %1 (code: 0x%2)").
                                     arg(reply->errorString()).
                                     arg(reply->error(), -1, 16));
        } else if (error == QModbusDevice::NoError) {
          ModbusHodlingRegsResponse(reply->serverAddress(), reply->result());
        }
        reply->deleteLater();
        this->exit();
      });
      this->exec();
    } else {
      delete reply; // broadcast replies return immediately
    }
  } else {
    emit DHSignal_ErrorOccured(tr("Read error: %1").arg(modbus_device_->errorString()));
  }
}

void DHController::ModbusWriteHoldingRegister(int slave_address,
                                              int start_address,
                                              QList<quint16> write_list) {
  if(!is_modbus_connected_) {
    return;
  }

  QModbusDataUnit write_unit(QModbusDataUnit::HoldingRegisters,
                             start_address,
                             static_cast<quint16>(write_list.size()));
  for(qsizetype index=0;index<write_list.size();index++) {
    write_unit.setValue(index, write_list.at(index));
  }

  QModbusReply *reply = modbus_device_->sendWriteRequest(write_unit, slave_address);
  if(reply) {
    if(!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, [this, reply]() {
        const auto error = reply->error();
        if (error == QModbusDevice::ProtocolError) {
          emit DHSignal_ErrorOccured(tr("Write error: %1 (Modbus exception: 0x%2)").
                                     arg(reply->errorString()).
                                     arg(reply->rawResult().exceptionCode(), -1, 16));
        } else if (error != QModbusDevice::NoError) {
          emit DHSignal_ErrorOccured(tr("Write error: %1 (code: 0x%2)").
                                     arg(reply->errorString()).
                                     arg(error, -1, 16));
        } else if (error == QModbusDevice::NoError) {
          ModbusHodlingRegsResponse(reply->serverAddress(), reply->result());
        }
        reply->deleteLater();
        this->exit();
      });
      this->exec();
    }
    else {
      reply->deleteLater();
    }
  }
  else {
    emit DHSignal_ErrorOccured(tr("Write error: %1").
                               arg(modbus_device_->errorString()));
  }
}

void DHController::ModbusHodlingRegsResponse(const int slave_address,
                                             const QModbusDataUnit unit) {
  if(slave_address == device_rgi->slave_address_) {
    device_rgi->UpdateData(unit);
  }
}

void DHController::ModbusQueueClear() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  send_func_queue_.clear();
  mutex_.unlock();
}

bool DHController::ModbusQueueIsEmpty() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  bool is_queue_empty = send_func_queue_.isEmpty();
  mutex_.unlock();
  return is_queue_empty;
}

ModbusFunc DHController::ModbusQueueGetFront() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  ModbusFunc func = send_func_queue_.front();
  mutex_.unlock();
  return func;
}

void DHController::ModbusQueuePopFront() {
  mutex_.tryLock(MUTEX_LOCK_TIMEOUT);
  send_func_queue_.pop_front();
  mutex_.unlock();
}

void DHController::ModbusQueueHandle() {
  if(ModbusQueueIsEmpty()) {
    return;
  }

  ModbusFunc send_func = ModbusQueueGetFront();
  ModbusSendFunction(send_func);
  ModbusQueuePopFront();
}

void DHController::ModbusSendFunction(ModbusFunc func_code) {
  if(!IsValueInRange(SLAVE_ADDRESS_MAX, SLAVE_ADDRESS_MIN, func_code.slave_address)) {
    return;
  }

  switch (func_code.func_code) {
    case FuncCode::kFuncReadHoldingRegs:
      ModbusReadHodlingRegister(func_code.slave_address,
                                func_code.start_address,
                                func_code.amount);
      break;
    case FuncCode::kFuncWriteHoldingRegs:
      ModbusWriteHoldingRegister(func_code.slave_address,
                                 func_code.start_address,
                                 func_code.value);
      break;
  }
}

void DHController::RGI_Init() {
  device_gri_address_ = 1;
  device_rgi = new DH_RGI(device_gri_address_);
}

void DHController::RGI_Collect_info() {
  ModbusSendFunction(device_rgi->GetDeviceFeedbackInfo());
}

void DHController::RGI_Uinit() {
  delete device_rgi;
}
}
