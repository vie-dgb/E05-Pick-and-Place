#ifndef FLEXIBLEFEED_H
#define FLEXIBLEFEED_H

#include <QObject>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QUrl>
#include <QTimer>

#define MODBUS_RESPONSE_TIME        1000
#define MODBUS_NUM_OF_RETRIES       3
#define FEEDER_MAX_GROUP_NUM        5
#define FEEDER_GROUP_SIZE           14

class FlexibleFeed : public QObject
{
  Q_OBJECT
public:
  enum FeedAddress : int {
    VersionNumber = 0,
    InputPointNumber = 1,
    IndicatorLampStatus = 2,
    MotorState = 3,
    AlarmStatus = 4,
    CurrentTemperature = 5,
    LightMode = 6,
    LightSwitch = 7,
    LightLuminace = 8,
    LightDelayOffTime = 9,
    ModeNumber = 10,
    GroupNumber = 11,
    CommunicateMode = 12,
    VibrateMode = 13,
    Pattern_1 = 14
  };

  enum CommunicationMode : int {
    Stop = 0,
    UpperLeft = 1,
    FallingRise = 2,
    UpperRight = 3,
    LeftSide = 4,
    Scatter = 5,
    LeftAndRightSide = 6,
    HighAndLow = 7,
    Right = 8,
    BottomLeft = 9,
    Under = 10,
    LowerRight = 11
  };

  enum VibrationMode : int {
    MasterSingleTest = 0,
    Last = 1
  };

  enum LightSourceMode : int {
    Manual = 0,
    Follow
  };

  struct VibrateGroup {
  public:
    uint16_t vibrationTime = 0;
    uint16_t interval = 0;
    uint16_t motor1Frequency = 0;
    uint16_t motor2Frequency = 0;
    uint16_t motor3Frequency = 0;
    uint16_t motor4Frequency = 0;
    uint16_t motor1Amplitude = 0;
    uint16_t motor2Amplitude = 0;
    uint16_t motor3Amplitude = 0;
    uint16_t motor4Amplitude = 0;
    uint16_t motor1Direction = 0;
    uint16_t motor2Direction = 0;
    uint16_t motor3Direction = 0;
    uint16_t motor4Direction = 0;
  };

  struct VibratePattern
  {
  public:
    uint16_t ouputTime;
    VibrateGroup group[FEEDER_MAX_GROUP_NUM];
  };

  struct FeederData {
  public:
    uint16_t versionNumber;
    uint16_t inputPointStatus;
    uint16_t indicatorLampStatus;
    uint16_t motorState;
    uint16_t alarmStatus;
    uint16_t currentTemperature;
    uint16_t lightSourceMode;
    uint16_t lightSourceSwitch;
    uint16_t lightSourceLuminance;
    uint16_t lightDelayOffTme;
    uint16_t currentModeNumber;
    uint16_t currentGroupNumber;
    uint16_t communicateModeNumber;
    uint16_t vibrationMode;
    VibratePattern pattern;
  };

  FlexibleFeed(QObject *parent = nullptr);
  ~FlexibleFeed();

  void FeederConnect(QString deviceUrl);
  void FeederConnect(QString host, int port);
  void FeederDisconnect();
  bool isFeederConnected();
  FeederData getFeederData();

  void writeLightControlMode(LightSourceMode lightMode);
  void writeLightSwitch(bool switchState);
  void writeLightLuminance(int value);
  void writeLightDelayOffTime(int time);
  void writeCommunicateMode(CommunicationMode comMode);
  void writeVibrationMode(VibrationMode vibraMode);
  void writePattern(VibratePattern pattern);
  void readFeederCurrentStatus();
  void readFeederLightData();
  void readFeederCurrentMode();
  void readFeederPattern();

private:
  void readFeederInitValue();
  void writeHoldingRegister(QModbusDataUnit writeUnit);
  void writeHoldingRegister(int address, quint16 value);
  void readHoldingRegister(int address, int amount);
  void readHoldingRegister(int address);
  bool updateFeederData(const QModbusDataUnit unit);
  void setFeederData(int address, uint16_t value);
  void setFeederGroupData(int address, uint16_t value);

signals:
  void FeederConnected();
  void FeederDisconnected();
  void FeederConnecting();
  void FeederConnectFail();
  void FeederConnectInitFail(QString msg);
  void FeederWriteError(QString msg);
  void FeederWriteResponseError(QString msg);
  void FeederReadError(QString msg);
  void FeederReadResponseError(QString msg);
  void FeederReadDone(FeederData newData);

private slots:
  void modbusChangedState(QModbusDevice::State state);
  void modbusReadReady();

private:
  QString serverHost;
  int serverPort;
  QModbusClient *modbusDevice;
  bool connected;
  FeederData m_data;
};

#endif // FLEXIBLEFEED_H
