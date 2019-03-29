#ifndef QFIRMATACONNECTION_H
#define QFIRMATACONNECTION_H

#include <QObject>
#include <QString>
#include <QIODevice>
#include <QJsonObject>
#include "private/qfirmata_p.h"
/********************************************************
 * QFirmataConnectionInterface defines the interface.
 * QFirmataConnection is a convenience implementation.
 *
 * QFirmataConnection supports 3 QIODevices.
 * To exetend the devices supported, or implement a custom
 * device. Just extend the QFirmataInterface.
 *
 * QFirmataConnection will work for many common use cases.
 * Primarily, it is used to wrap a QSerialPort connection.
 * This connection is assumed (USB)->UART connection of
 * the arduino board.
 * ********************************************************/

class QFirmataConnectionInterface
{
protected:
    explicit QFirmataConnectionInterface(){}

public:
    enum class IoMode {
        Input = 0x00,   //! Digital input
        Output = 0x01,  //! Digital output
        Analog = 0x02,  //! Analog input
        PWM = 0x03,     //! PWM output
        Servo = 0x04,   //! RC servo output
        Shift = 0x05,   //! Shift register
        I2C = 0x06,     //! I²C bus
        OneWire = 0x07, //! OneWire bus,
        Stepper = 0x08, //! Stepper motor
        Encoder = 0x09, //! Encoder input
        Serial = 0x0a,  //! Serial port
        PullUp = 0x0b,   //! Digital input with internal pull-ups
        Am2302 = 0x0c //! Am2302 temperature and humidity sensor
    };
    // FIRMATA Connection Interface

    virtual bool isAvailable() const = 0;

    virtual QString statusText() const = 0;

        // Low level Firmata messages. These are typically called by Firmata
    //! Write a 14bit analog value
    virtual void writeAnalogPin(quint8 pin, quint16 value) = 0;

    //! Write the value of a digital pin
    virtual void writeDigitalPin(quint8 pin, bool value) = 0;

    //! Enable/disable analog pin value
    virtual void enableAnalogPin(quint8 pin, bool enable) = 0;

    //! Enable disable digital port value
    virtual void enableDigitalPort(quint8 port, bool enable) = 0;

    //! Request the device to report its protocol version
    virtual void  requestProtocolVersion() = 0;

    //! Request the device to report its protocol version
    virtual void requestCapabilitiesMap() = 0;

    //! Set the mode of a Pin
    virtual void setPinMode(quint8 pin, QFirmataConnectionInterface::IoMode mode) = 0;

    //! Send a SysEx command
    virtual void writeSysexMessage(QByteArray data) = 0;

    //! Firmata Signals
    //! An analog message was just received
    virtual void analogValueRead(quint8 channel, quint16 value )= 0;
    //! A digital message was just received
    virtual void digitalMessageRead(quint8 port, quint8 value) = 0;

    //! Individual digital pin value changed
    virtual void digitalPinChanged(quint8 pin, bool value) = 0;

    //! A SysEx command was just received
    virtual void sysexMessageRead(const QByteArray &data) = 0;

    //! Protocol version was just received
    virtual void protocolVersionRead(int major, int minor) = 0;

    //! Capabilities was just received
    void capabilitiesRead(QJsonObject obj);

    //! Connection Signals
    virtual void availabilityChanged(bool available) = 0;
    virtual void statusTextChanged(QString text) = 0;
    virtual void firmataWriteByteArray(QByteArray message) = 0;
    virtual void firmataWriteCharArray(const char *data, quint64 len) = 0;

    ~QFirmataConnectionInterface(){}

};

Q_DECLARE_INTERFACE(QFirmataConnectionInterface, "QFirmataConnectionInterace")


class QSerialPort;
class QTcpSocket;
class QWebSocket;
class QFIRMATA_EXPORT QFirmataConnection : public QObject
{
   Q_OBJECT
   //Q_ENUMS(QFirmataConnectionInterface::IoMode)
public:
    QFirmataConnection(QSerialPort & connection,QObject * parent = nullptr);
    QFirmataConnection(QTcpSocket & connection, QObject * parent = nullptr);
    QFirmataConnection(QWebSocket & connection, QObject * parent = nullptr);

    bool isAvailable() const;
    int  channel() const;
    void assignChannel(int channel);
    void begin();
    bool waitRead(int);
        // Low level Firmata messages. These are typically called by Firmata

signals:

    void wroteByteArray(QByteArray message);
    void channelReadyRead(int channel,QByteArray message);
    void connected();
public slots:

    //! Log and internal connection to each output write
    void firmataWriteByteArray(QByteArray data);
    void firmataWriteCharArray(const char *data, quint64 len);
    void newMessage(int channel, QJsonObject message);


    void readDHTSensor(qint8 pin, quint8 type);

    void setRelay(qint8 id, bool enabled);

    void readRelayStatus(qint8 relay);
    //! Write a 14bit analog value
    void writeAnalogPin(quint8 pin, quint16 value);

    //! Write the value of a digital pin
    void writeDigitalPin(quint8 pin, bool value);

    //! Enable/disable analog pin value
    void enableAnalogPin(quint8 pin, bool enable);

    //! Enable disable digital port value
    void enableDigitalPort(quint8 port, bool enable);

    //! Request the device to report its protocol version
    void requestProtocolVersion();

    //! Request the device to report its capabilities
    void  requestCapabilitiesMap();

    //! Set the mode of a Pin
    void setPinMode(quint8 pin, QFirmataConnectionInterface::IoMode mode);

    //! Send a SysEx command
    void writeSysexMessage(quint8 command, QByteArray message);

signals:
    //! Firmata Signals
    //! An analog message was just received
    void analogValueRead(quint8 channel, quint16 value);

    //! A digital message was just received
    void digitalMessageRead(quint8 port, quint8 value);

    //! Individual digital pin value changed
    void digitalPinChanged(quint8 pin, bool value);

    //! A SysEx command was just received
    void sysexMessageRead(const QByteArray &data);

    //! Protocol version was just received
    void protocolVersionRead(int major, int minor);

    //! Capabilities was just received
    void capabilitiesRead(QJsonObject obj);

    //! Connection Signals
    void availabilityChanged(bool available);

private slots:

    // process incomming messages
    void processUartMessage();
    void processSysexResponse(QByteArray message);

private:
    QObject &   mConnectedDevice;
    int         mDeviceType;
    int         mSecondsWaited;
    int         mAssignedChannel;
};

#endif // QFIRMATACONNECTION_H
