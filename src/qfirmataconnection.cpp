#include "qfirmataconnection.h"
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QByteArray>
#include <QJsonArray>
#include <QTimer>
static const char SERIAL_DEVICE = 0x00;
QFirmataConnection::QFirmataConnection(QSerialPort &connection, QObject *parent) :
    QObject(parent), mConnectedDevice(connection), mDeviceType(SERIAL_DEVICE), mSecondsWaited(0)
{
    connect(&connection,&QSerialPort::readyRead, this, &QFirmataConnection::processUartMessage);
}

void QFirmataConnection::assignChannel(int channel)
{
    mAssignedChannel = channel;
}


void QFirmataConnection::begin()
{
    if(mDeviceType == SERIAL_DEVICE){
        auto serialp = qobject_cast<QSerialPort*>(&mConnectedDevice);
        if(serialp->waitForReadyRead(10000))
            emit connected();
    }
}

int QFirmataConnection::channel() const
{
    return mAssignedChannel;
}

bool QFirmataConnection::isAvailable() const
{
    bool avaialable(false);
    switch(mDeviceType){
        case SERIAL_DEVICE:
        {
            QSerialPort * device = qobject_cast<QSerialPort*>(&mConnectedDevice);
            avaialable  = device->isOpen();
            break;
        }
        default: break;
    }
    return avaialable;
}

    // Low level Firmata messages. These are typically called by Firmata

//! Write a 14bit analog value
void QFirmataConnection::writeAnalogPin(quint8 pin, quint16 value)
{
    QByteArray out;
    out.append(cmdPin(ANALOG_MESSAGE, pin));
    out.append(lsb14(value));
    out.append(msb14(value));
    firmataWriteByteArray(out);

}

void QFirmataConnection::readDHTSensor(qint8 pin, quint8 type)
{
    QByteArray out;
    out.append(START_SYSEX);
    out.append(STATUS_QUERY);
    out.append(pin);
    out.append(type);
    out.append(END_SYSEX);
    firmataWriteByteArray(out);
}

void QFirmataConnection::readRelayStatus(qint8 relay)
{
    QByteArray out;
    out.append(START_SYSEX);
    out.append(STATUS_QUERY);
    out.append(relay);
    out.append(END_SYSEX);
    firmataWriteByteArray(out);
}


void QFirmataConnection::setRelay(qint8 id, bool enabled)
{
    QByteArray out;
    out.append(START_SYSEX);
    out.append(SET_RELAY);
    out.append(id);
    out.append(qint8(enabled ? 1:0));
    out.append(END_SYSEX);
    firmataWriteByteArray(out);
}

//! Write the value of a digital pin
void QFirmataConnection::writeDigitalPin(quint8 pin, bool value)
{

}

//! Enable/disable analog pin value
void QFirmataConnection::enableAnalogPin(quint8 pin, bool enable)
{

}

//! Enable disable digital port value
void QFirmataConnection::enableDigitalPort(quint8 port, bool enable)
{

}

void QFirmataConnection::newMessage(int channel, QJsonObject message)
{
    if(message.value("type").toInt() == CAPABILITY_RESPONSE){
        QJsonArray pins = message.value("pins").toArray();
        for(int index(0); index < pins.count(); index++){
            QJsonArray available = pins.at(index).toArray();
            if(available.count() >= 2){
                for(int i2(0); i2 < available.count()/2;){
                    //qDebug() << "("<< index << ") Pin Mode " << digitalPinModeToString(available.at(i2).toInt()) <<": " << available.at(i2+1).toInt();
                    i2 += 2;
                }
            }
        }
    }
    else{
        //qDebug() << "Received message: " << message;
    }
}

//! Request the device to report its protocol version
void  QFirmataConnection::requestProtocolVersion()
{
    QByteArray out;
    out.append(START_SYSEX);
    out.append(QUERY_FIRMWARE);
    out.append(END_SYSEX);
    firmataWriteByteArray(out);
}

void QFirmataConnection::requestCapabilitiesMap()
{
    QByteArray out;
    out.append(START_SYSEX);
    out.append(CAPABILITY_QUERY);
    out.append(END_SYSEX);
    firmataWriteByteArray(out);
}

//! Set the mode of a Pin
void QFirmataConnection::setPinMode(quint8 pin, QFirmataConnectionInterface::IoMode mode)
{

}

//! Send a SysEx command
void QFirmataConnection::writeSysexMessage(quint8 command, QByteArray message)
{
    QByteArray data;
    data.append(START_SYSEX);
    data.append(command);
    for(int index(0); index < message.count(); index++){
        quint8 byte;
        auto value = message.at(index);
        byte = (value & 0xFF);
        data.append(byte);
    }
    data.append(END_SYSEX);
    firmataWriteByteArray(data);
}



void QFirmataConnection::processUartMessage()
{
    QSerialPort * device = qobject_cast<QSerialPort*>(&mConnectedDevice);
    QByteArray data = device->readAll();
    emit channelReadyRead(mAssignedChannel,data);

}

void QFirmataConnection::processSysexResponse(QByteArray message)
{

}

//! Log and internal connection to each output write
void QFirmataConnection::firmataWriteByteArray(QByteArray message)
{
    switch(mDeviceType){
        case SERIAL_DEVICE:
        {
            QSerialPort * device = qobject_cast<QSerialPort*>(&mConnectedDevice);
            if(device){
                device->write(message.data(),message.length());
                device->waitForBytesWritten(2000);
            }
            break;
        }
        default: break;
    }
}
void QFirmataConnection::firmataWriteCharArray(const char *data, quint64 len)
{
    switch(mDeviceType){
        case SERIAL_DEVICE:
        {
            QSerialPort * device = qobject_cast<QSerialPort*>(&mConnectedDevice);
            if(device){
                device->write(data,len);
            }
            break;
        }
        default: break;
    }
}


bool QFirmataConnection::waitRead(int millis)
{
    if(mDeviceType == SERIAL_DEVICE)
    {
        QSerialPort * device = qobject_cast<QSerialPort*>(&mConnectedDevice);
        return device->waitForReadyRead(millis);
    }
    return false;
}
