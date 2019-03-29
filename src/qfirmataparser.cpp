#include "qfirmataparser.h"
#include <QDebug>
QFirmataParser::QFirmataParser(QObject *parent) : QObject(parent)
{
    QFirmataParseContext * c0 = new QFirmataParseContext(500);
    mChannels.insert(0,c0);

}

void QFirmataParser::appendData(QByteArray data) // append to channel zero
{
    QFirmataParseContext * context = mChannels.value(0);
    context->appendData(data);
    bool parsing(true);
    int result(0);
    while(parsing){
        result = context->parseNextByte();
        switch(result)
        {
            case 0: parsing = false; break; // no message parsed, reached end of data, no error
            case 1: emit messageReceived(0,context->mCurrentMessage); context->clearMessage();break;
            default: qDebug() << "Errored on parsing" << context->mCurrentMessage; context->clearError();break; // error
        }
    }
}

void QFirmataParser::appendDataChannel(int channel, QByteArray data)
{
    if(mChannels.contains(channel)){
        QFirmataParseContext * context = mChannels.value(channel);
        context->appendData(data);
        bool parsing(true);
        while(parsing){
            int result(context->parseNextByte());
            switch(result)
            {
                case 0:
                parsing = true;
                break; // continue parsing
            case 1:
                parsing = true;
                emit messageReceived(channel,context->mCurrentMessage);
                context->clearMessage();
                break;
            case 2: parsing = false;
                break; // no message parsed, reached end of data, no error
            default:
                qDebug() << "Errored on " << result << "  parsing" << context->mCurrentMessage;
                context->clearError();
                parsing = (context->mData.length() > 0);
                break; // error
            }
        }
    }
}


void QFirmataParser::clearAll(int channel)
{
    if(mChannels.contains(channel)){
        mChannels.value(channel)->clearContext();
    }
}


void QFirmataParser::closeChannel(int channel)
{
    if(mChannels.contains(channel)){
        QFirmataParseContext * context = mChannels.take(channel);
        delete context;
    }
}


void QFirmataParser::openChannel(int channel)
{
    if(mChannels.contains(channel)){
        return;
    }
   QFirmataParseContext* context = new QFirmataParseContext(500);
    mChannels.insert(channel,context);
}

void QFirmataParser::requestChannelList()
{
    emit channelList(mChannels.keys());
}

void QFirmataParser::resetParse(int channel)
{

}


QFirmataParser::~QFirmataParser()
{
    QList<QFirmataParseContext*> buffer_list(mChannels.values());
    for(int index(0); index < buffer_list.length(); index++){
        delete buffer_list.takeFirst();
    }
}
