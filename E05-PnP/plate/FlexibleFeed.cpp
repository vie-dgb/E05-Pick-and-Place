#include "FlexibleFeed.h"
#include "qvariant.h"

FlexibleFeed::FlexibleFeed(QObject *parent)
{
  modbusDevice = new QModbusTcpClient(parent);
  connected = false;

  // init slot connect
  connect(modbusDevice, SIGNAL(stateChanged(QModbusDevice::State)),
          this, SLOT(modbusChangedState(QModbusDevice::State)));
  connect(&holdTimer, SIGNAL(timeout()), this, SLOT(holdTimerTimeout()));
}

FlexibleFeed::~FlexibleFeed() {
  if(modbusDevice) {
    modbusDevice->disconnectDevice();
  }
  delete modbusDevice;
}

void FlexibleFeed::FeederConnect(QString deviceUrl) {
  if(modbusDevice->state() != QModbusDevice::State::UnconnectedState) {
    return;
  }

  if(deviceUrl.isEmpty()) {
    return;
  }

  // set modbus host address and port
  const QUrl url = QUrl::fromUserInput(deviceUrl);
  serverHost = url.host();
  serverPort = url.port();
  modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, serverHost);
  modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, serverPort);

  // set modbus connect timeout and number of retries time
  modbusDevice->setTimeout(MODBUS_RESPONSE_TIME);
  modbusDevice->setNumberOfRetries(MODBUS_NUM_OF_RETRIES);

  // connect, if connect fail emit corresponding signal
  if (!modbusDevice->connectDevice()) {
    emit FeederConnectInitFail(tr("Connect failed: %1").arg(modbusDevice->errorString()));
  }
}

void FlexibleFeed::FeederConnect(QString host, int port) {
  if(modbusDevice->state() != QModbusDevice::State::UnconnectedState) {
    return;
  }

  if(host.isEmpty()) {
    return;
  }

  // set modbus host address and port
  modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
  modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

  // set modbus connect timeout and number of retries time
  modbusDevice->setTimeout(MODBUS_RESPONSE_TIME);
  modbusDevice->setNumberOfRetries(MODBUS_NUM_OF_RETRIES);

  // connect, if connect fail emit corresponding signal
  if (!modbusDevice->connectDevice()) {
    emit FeederConnectInitFail(tr("Connect failed: %1").arg(modbusDevice->errorString()));
  }
}

void FlexibleFeed::FeederDisconnect() {
  modbusDevice->disconnectDevice();
}

bool FlexibleFeed::isFeederConnected() {
  return connected;
}

FlexibleFeed::FeederData FlexibleFeed::getFeederData() {
  return m_data;
}

void FlexibleFeed::writeLightControlMode(LightSourceMode lightMode) {
  writeHoldingRegister(static_cast<int>(FeedAddress::LightMode), static_cast<quint16>(lightMode));
}

void FlexibleFeed::writeLightSwitch(bool switchState) {
  writeHoldingRegister(static_cast<int>(FeedAddress::LightSwitch), static_cast<quint16>((switchState) ? 1 : 0));
}

void FlexibleFeed::writeLightLuminance(int value) {
  writeHoldingRegister(static_cast<int>(FeedAddress::LightLuminace), static_cast<quint16>(value));
}

void FlexibleFeed::writeLightDelayOffTime(int time) {
  writeHoldingRegister(static_cast<int>(FeedAddress::LightDelayOffTime), static_cast<quint16>(time));
}

void FlexibleFeed::writeCommunicateMode(CommunicationMode comMode) {
  writeHoldingRegister(static_cast<int>(FeedAddress::CommunicateMode), static_cast<quint16>(comMode));
}

void FlexibleFeed::writeVibrationMode(VibrationMode vibraMode) {
  writeHoldingRegister(static_cast<int>(FeedAddress::VibrateMode), static_cast<quint16>(vibraMode));
}

void FlexibleFeed::writePattern(VibratePattern pattern) {
  int amount = 14*FEEDER_MAX_GROUP_NUM + 1;
  QModbusDataUnit writeUnit(QModbusDataUnit::RegisterType::HoldingRegisters,
                            FeedAddress::Pattern_1,
                            amount);

  writeUnit.setValue(0, pattern.ouputTime);
  for(int groupIndex=0;groupIndex<FEEDER_MAX_GROUP_NUM;groupIndex++) {
    writeUnit.setValue(1 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].vibrationTime);
    writeUnit.setValue(2 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].interval);
    writeUnit.setValue(3 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor1Frequency);
    writeUnit.setValue(4 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor2Frequency);
    writeUnit.setValue(5 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor3Frequency);
    writeUnit.setValue(6 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor4Frequency);
    writeUnit.setValue(7 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor1Amplitude);
    writeUnit.setValue(8 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor2Amplitude);
    writeUnit.setValue(9 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor3Amplitude);
    writeUnit.setValue(10 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor4Amplitude);
    writeUnit.setValue(11 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor1Direction);
    writeUnit.setValue(12 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor2Direction);
    writeUnit.setValue(13 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor3Direction);
    writeUnit.setValue(14 + (groupIndex*FEEDER_GROUP_SIZE), pattern.group[groupIndex].motor4Direction);
  }

  writeHoldingRegister(writeUnit);
}

void FlexibleFeed::readFeederCurrentStatus() {
  readHoldingRegister(static_cast<int>(FeedAddress::VersionNumber), 6);
}

void FlexibleFeed::readFeederLightData() {
  readHoldingRegister(static_cast<int>(FeedAddress::LightMode), 4);
}

void FlexibleFeed::readFeederCurrentMode() {
  readHoldingRegister(static_cast<int>(FeedAddress::ModeNumber), 4);
}

void FlexibleFeed::readFeederPattern() {
  readHoldingRegister(static_cast<int>(FeedAddress::Pattern_1), 71);
}

void FlexibleFeed::readFeederInitValue() {
  // read all value from feeder
  readHoldingRegister(static_cast<int>(FeedAddress::VersionNumber), 85);
}

void FlexibleFeed::writeHoldingRegister(QModbusDataUnit writeUnit) {
  if(!connected) {
    return;
  }

  QModbusReply *reply = modbusDevice->sendWriteRequest(writeUnit, serverPort);
  if(reply) {
    if(!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, [this, reply]() {
        const auto error = reply->error();
        if (error == QModbusDevice::ProtocolError) {
          emit FeederWriteResponseError(tr("Write response error: %1 (Modbus exception: 0x%2)")
                                            .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        } else if (error != QModbusDevice::NoError) {
          emit FeederWriteResponseError(tr("Write response error: %1 (code: 0x%2)")
                                            .arg(reply->errorString()).arg(error, -1, 16));
        } else if (error == QModbusDevice::NoError) {
          updateFeederData(reply->result());
          emit FeederReadDone(m_data);
        }
        reply->deleteLater();
      });
    }
    else {
      reply->deleteLater();
    }
  }
  else {
    emit FeederWriteError(tr("Write error: %1").arg(modbusDevice->errorString()));
  }
}

void FlexibleFeed::writeHoldingRegister(int address, quint16 value) {
  QModbusDataUnit writeUnit(QModbusDataUnit::RegisterType::HoldingRegisters, address, 1);
  writeUnit.setValue(0, value);
  writeHoldingRegister(writeUnit);
}

void FlexibleFeed::readHoldingRegister(int address, int amount) {
  if(!connected) {
    return;
  }

  auto *reply = modbusDevice->sendReadRequest(
      QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, address, amount),
      serverPort);

  if(reply) {
    if(!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, &FlexibleFeed::modbusReadReady);
    }
    else {
      delete reply;
    }
  }
  else {
    emit FeederReadError(tr("Read error: %1").arg(modbusDevice->errorString()));
  }
}

void FlexibleFeed::readHoldingRegister(int address) {
  readHoldingRegister(address, 1);
}

bool FlexibleFeed::updateFeederData(const QModbusDataUnit unit) {
  if(unit.valueCount() < 0) {
    return false;
  }

  for (qsizetype i = 0, total = unit.valueCount(); i < total; ++i) {
    setFeederData(static_cast<FeedAddress>(unit.startAddress() + i), unit.value(i));
  }
  return true;
}

void FlexibleFeed::setFeederData(int address, uint16_t value) {
  switch (address) {
    case VersionNumber:
      m_data.versionNumber = value;
      return;
    case InputPointNumber:
      m_data.inputPointStatus = value;
      return;
    case IndicatorLampStatus:
      m_data.indicatorLampStatus = value;
      return;
    case MotorState:
      m_data.motorState = value;
      return;
    case AlarmStatus:
      m_data.alarmStatus = value;
      return;
    case CurrentTemperature:
      m_data.currentTemperature = value;
//      qDebug() << "Current temperature:" << value;
      return;
    case LightMode:
      m_data.lightSourceMode = value;
      return;
    case LightSwitch:
      m_data.lightSourceSwitch = value;
      return;
    case LightLuminace:
      m_data.lightSourceLuminance = value;
      return;
    case LightDelayOffTime:
      m_data.lightDelayOffTme = value;
      return;
    case ModeNumber:
      m_data.currentModeNumber = value;
      return;
    case GroupNumber:
      m_data.currentGroupNumber = value;
      return;
    case CommunicateMode:
      m_data.communicateModeNumber = value;
      return;
    case VibrateMode:
      m_data.vibrationMode = value;
      return;
  }

  setFeederGroupData(address, value);
}

void FlexibleFeed::setFeederGroupData(int address, uint16_t value) {
  if(address == FeedAddress::Pattern_1) {
    m_data.pattern.ouputTime = value;
    return;
  }

  int groupIndex = address - (FeedAddress::Pattern_1 + 1); // start pattern at 14 + 1
  groupIndex /= FEEDER_GROUP_SIZE;
  int index = address - (FEEDER_GROUP_SIZE*(groupIndex+1) + 1);

  switch (index) {
    case 0:
      m_data.pattern.group[groupIndex].vibrationTime = value;
      return;
    case 1:
      m_data.pattern.group[groupIndex].interval = value;
      return;
    case 2:
      m_data.pattern.group[groupIndex].motor1Frequency = value;
      return;
    case 3:
      m_data.pattern.group[groupIndex].motor2Frequency = value;
      return;
    case 4:
      m_data.pattern.group[groupIndex].motor3Frequency = value;
      return;
    case 5:
      m_data.pattern.group[groupIndex].motor4Frequency = value;
      return;
    case 6:
      m_data.pattern.group[groupIndex].motor1Amplitude = value;
      return;
    case 7:
      m_data.pattern.group[groupIndex].motor2Amplitude = value;
      return;
    case 8:
      m_data.pattern.group[groupIndex].motor3Amplitude = value;
      return;
    case 9:
      m_data.pattern.group[groupIndex].motor4Amplitude = value;
      return;
    case 10:
      m_data.pattern.group[groupIndex].motor1Direction = value;
      return;
    case 11:
      m_data.pattern.group[groupIndex].motor2Direction = value;
      return;
    case 12:
      m_data.pattern.group[groupIndex].motor3Direction = value;
      return;
    case 13:
      m_data.pattern.group[groupIndex].motor4Direction = value;
      return;
  }
}

void FlexibleFeed::modbusChangedState(QModbusDevice::State state) {
  switch (state) {
    case QModbusDevice::State::ConnectedState:
      connected = true;
      emit FeederConnected();
      holdTimer.start(holdRefreshTime);
      readFeederInitValue();
      break;
    case QModbusDevice::State::UnconnectedState:
      if(!connected) {
        emit FeederConnectFail();
        return;
      }
      connected = false;
      emit FeederDisconnected();
      break;
    case QModbusDevice::State::ClosingState:
      holdTimer.stop();
      break;
    case QModbusDevice::State::ConnectingState:
      emit FeederConnecting();
      break;
  }
}

void FlexibleFeed::modbusReadReady() {
  auto reply = qobject_cast<QModbusReply *>(sender());
  if (!reply)
    return;

  if (reply->error() == QModbusDevice::NoError) {
    if(updateFeederData(reply->result())) {
      emit FeederReadDone(m_data);
    }
  } else if (reply->error() == QModbusDevice::ProtocolError) {
    emit FeederReadResponseError(tr("Read response error: %1 (Modbus exception: 0x%2)").
                                 arg(reply->errorString()).
                                 arg(reply->rawResult().exceptionCode(), -1, 16));
  } else {
    emit FeederReadResponseError(tr("Read response error: %1 (code: 0x%2)").
                                 arg(reply->errorString()).
                                 arg(reply->error(), -1, 16));
  }

  reply->deleteLater();
}

void FlexibleFeed::holdTimerTimeout() {
  if (modbusDevice->state() == QModbusDevice::State::ConnectedState) {
    readHoldingRegister(CurrentTemperature, 1);
  }
}
