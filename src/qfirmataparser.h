#ifndef QFIRMATAPARSER_H
#define QFIRMATAPARSER_H

#include <QObject>
#include <QJsonObject>
#include <QByteArray>
#include <QBuffer>
#include <QMap>
#include "private/qfirmata_p.h"
typedef  QMap<int,QFirmataParseContext*> ChannelBufferMap;
/*************************************************************
*   FirmataParser provides an asynchronous (Qt slot)
*  interface to a serial parse buffer. New bytes are loaded
* using the appendData slot. When a message is parsed,
* it the FirmataParser will notify the system using
* firmata message received. In the case of an error, the
* parseErrror signal will be emitted instead.
*
* A Firmata message is pre-parsed and organized using a
*  json object. Json objects make it easy to form and store
* the associative maps required for nested messaging.
* For instance the CapabilitesMap stores an array of pin
* defintions. Each pin definitions can have multiple modes
* and configurations. So they are provided as internel
* objects.
*
* The Firmata Parser can also operate on multiple "channels"
* meaning that most applications only need one firmata parser.
* *************************************************************/
class QFIRMATA_EXPORT QFirmataParser : public QObject
{
    Q_OBJECT
public:
    explicit QFirmataParser(QObject *parent = nullptr);
    ~QFirmataParser();
signals:
    void messageReceived(int channel,const QJsonObject & message);
    void errorParsingMessage(const QJsonObject& incomplete_message, int error_type);

    // channel signals
    void channelOpened(int channel);
    void channelClosed(int channel);
    void channelList(QList<int> channels);

public slots:
    void appendData(QByteArray data); // defaults to channel zero
    void appendDataChannel(int channel, QByteArray data);
    void clearAll(int channel);
    void resetParse(int channel);

    // channel functions
    void openChannel(int channel);
    void closeChannel(int channel);
    void requestChannelList();

protected:
    // first parse start
    // then parse command
    // after command is found, parse the complete command
    ChannelBufferMap mChannels;
private:
    QFirmataParser operator =(const QFirmataParser& right){Q_UNUSED(right); return *this;}
    QFirmataParser(const QFirmataParser& right):QObject(){Q_UNUSED(right);}
};

#endif // QFIRMATAPARSER_H
