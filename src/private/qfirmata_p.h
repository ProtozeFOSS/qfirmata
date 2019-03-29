#ifndef FIRMATA_H
#define FIRMATA_H
#include <QDebug>
#include <QObject>
#if defined(QFIRMATA_LIBRARY)
#  define QFIRMATA_EXPORT Q_DECL_EXPORT
#else
#  define QFIRMATA_EXPORT Q_DECL_IMPORT
#endif
static const quint8
    CMD_ANALOG_IO = 0xe0,
    CMD_DIGITAL_IO = 0x90,
    CMD_ANALOG_REPORT = 0xc0,
    CMD_DIGITAL_REPORT = 0xd0,
    CMD_SYSEX_START = 0xf0,
    CMD_SYSEX_END = 0XF7,
    CMD_SET_PINMODE = 0xf4,
    CMD_SET_DIGITAL_PIN = 0xf5,
    CMD_PROTOCOL_VERSION = 0xf9,
    SYSEX_EXTENDED_ANALOG = 0x6f,
    ANALOG_MAPPING_QUERY = 0x69,
    ANALOG_MAPPING_RESPONSE = 0x6A,
    ANALOG_MESSAGE = 0xE0,
    CAPABILITY_QUERY = 0x6B,
    CAPABILITY_RESPONSE = 0x6C,
    DIGITAL_MESSAGE = 0x90,
    END_SYSEX = 0xF7,
    EXTENDED_ANALOG = 0x6F,
    I2C_CONFIG = 0x78,
    I2C_REPLY = 0x77,
    I2C_REQUEST = 0x76,
    I2C_READ_MASK = 0x18,
    I2C_END_TX_MASK = 0x40,
    ONEWIRE_CONFIG_REQUEST = 0x41,
    ONEWIRE_DATA = 0x73,
    ONEWIRE_DELAY_REQUEST_BIT = 0x10,
    ONEWIRE_READ_REPLY = 0x43,
    ONEWIRE_READ_REQUEST_BIT = 0x08,
    ONEWIRE_RESET_REQUEST_BIT = 0x01,
    ONEWIRE_SEARCH_ALARMS_REPLY = 0x45,
    ONEWIRE_SEARCH_ALARMS_REQUEST = 0x44,
    ONEWIRE_SEARCH_REPLY = 0x42,
    ONEWIRE_SEARCH_REQUEST = 0x40,
    ONEWIRE_WITHDATA_REQUEST_BITS = 0x3C,
    ONEWIRE_WRITE_REQUEST_BIT = 0x20,
    PIN_MODE = 0xF4,
    PIN_STATE_QUERY = 0x6D,
    PIN_STATE_RESPONSE = 0x6E,
    PING_READ = 0x75,
    PULSE_IN = 0x74,
    PULSE_OUT = 0x73,
    QUERY_FIRMWARE = 0x79,
    REPORT_ANALOG = 0xC0,
    REPORT_DIGITAL = 0xD0,
    REPORT_VERSION = 0xF9,
    SAMPLING_INTERVAL = 0x7A,
    SERVO_CONFIG = 0x70,
    SERIAL_MESSAGE = 0x60,
    SERIAL_CONFIG = 0x10,
    SERIAL_WRITE = 0x20,
    AM2032_READ = 0x12,
    STATUS_QUERY = 0x64,
    SET_RELAY = 0xA4,
    SERIAL_READ = 0x30,
    SERIAL_REPLY = 0x40,
    SERIAL_CLOSE = 0x50,
    SERIAL_FLUSH = 0x60,
    SERIAL_LISTEN = 0x70,
    START_SYSEX = 0xF0,
    STEPPER = 0x72,
    STRING_DATA = 0x71,
    INTERNAL_RESTART = 0xFD,
    SYSTEM_HALT = 0xFE,
    SYSTEM_RESET = 0xFF;

static const char* DIGITAL_PIN_MODE_FIRMATA[] = {"INPUT","OUTPUT","ANALOG","PWM","SERVO","I2C","ONEWIRE","STEPPER",
                                                 "ENCODER","SERIAL","PULLUP",  "USER_DEFINED","AM2302","ERROR"};
static const quint8 EXTENDED_COMMANDS_FIRMATA[] = { SERVO_CONFIG ,SERIAL_MESSAGE, SERIAL_CONFIG,
                                                                                                                    SERIAL_WRITE, AM2032_READ,SERIAL_READ,
                                                                                                                    SERIAL_REPLY,SERIAL_CLOSE,SERIAL_FLUSH,
                                                                                                                    SERIAL_LISTEN,START_SYSEX,STEPPER,STRING_DATA};
static const unsigned int MAX_PIN_COUNT = 128;
static bool checkInExtendedCommands(quint8 command)
{
    for(quint32 index(0); index < sizeof(EXTENDED_COMMANDS_FIRMATA); index++){
        if(command == EXTENDED_COMMANDS_FIRMATA[index]){
            return true;
        }
    }
    return false;
}
// Established Convenience Commands
static const unsigned int COMMAND_QUERY_FIRMWARE[] = {START_SYSEX,QUERY_FIRMWARE,END_SYSEX};
static const unsigned int COMMAND_QUERY_CAPABILITIES[] = {START_SYSEX,CAPABILITY_QUERY,END_SYSEX};
static const unsigned int COMMAND_QUERY_ANALOG_MAP[] = {START_SYSEX,ANALOG_MAPPING_QUERY,END_SYSEX};

inline uint8_t cmdPin(uint8_t cmd, uint8_t pin) {
    Q_ASSERT((cmd & 0x0f)==0);
    Q_ASSERT((pin & 0xf0)==0);
    return cmd | pin;
}
static const char* digitalPinModeToString(int mode){
    if(mode < 0 || mode > 12){
        return DIGITAL_PIN_MODE_FIRMATA[13];
    }
    return DIGITAL_PIN_MODE_FIRMATA[mode];
}
//! Return bits 0..6 of a 14 bit value
inline uint8_t lsb14(uint16_t v) { return v & 0x7f; }

//! Return bits 6..12 of a 14 bit value
inline uint8_t msb14(uint16_t v) { return (v>>7) & 0x7f; }

//! Extract a 14bit integer from 2 7-bit bytes
inline uint16_t unpack14(const uint8_t *data)
{
    return
        ((*(data+0) & 0x7f)<<0) |
        ((*(data+1) & 0x7f)<<7);
}

//! Extract a 28bit integer from 4 7-bit bytes
inline uint32_t unpack28(const uint8_t *data)
{
    return
        ((*(data+0) & 0x7f)<<0) |
        ((*(data+1) & 0x7f)<<7) |
        ((*(data+2) & 0x7f)<<14) |
        ((*(data+3) & 0x7f)<<21);
}
#include <QBuffer>
#include <QtEndian>
#include <QJsonObject>
#include <QJsonArray>

enum class ParserState {
    ExpectStart,
    ExpectCommand,
    ExpectStartOrCommand,
    ExpectParam1,
    ExpectParam2,
    ExpectSysexData,
    ExpectFirstSysEnd
};
typedef union {
    float value;
    quint8 data[4];
}FloatMask;

class QFirmataParseContext{
public:
    QFirmataParseContext(int reserve_size = 400) : mParseState(ParserState::ExpectStartOrCommand){
        mData.reserve(reserve_size);
        mBuffer.setBuffer(&mData);
        mBuffer.open(QIODevice::WriteOnly);
    }
    void  appendData(QByteArray data)
    {
        mBuffer.write(data);
    }
    quint8 peekNext(bool & error){
        quint8 next = 0x00;
        if(mData.length() > 0){
            next = mData.front();
        }
        else{
            error = true;
        }
        return next;
    }
    quint8 popNext(bool &error){
        quint8 next = 0x00;
        if(mData.length() > 0){
            next = mData.front();
            mData.remove(0,1);
        }
        else{
            error = true;
        }
        return next;
    }

    void clearContext(){

    }
    void clearMessage(){
        mParseState = ParserState::ExpectStartOrCommand;
        mCommand = 0;
        mCurrentMessage = QJsonObject();
        mParams = QJsonArray();
    }

    int parseNextByte()
    {
        int result(-1);
        bool error(false);
        quint8 byte (popNext(error));
        if(error){
            return 2; // out of data
        }
        switch(mParseState){
            case ParserState::ExpectStartOrCommand:
            if(byte == START_SYSEX){
                mParseState = ParserState::ExpectCommand;
                mCurrentMessage = QJsonObject();
                mParams = QJsonArray();
            }
            else if(byte >= 0x90){ // command
                mCommand = byte;
                prepareForCommand();
                mParseState = ParserState::ExpectParam1;
            }
            else{
                // check if it is an extended message
                if(checkInExtendedCommands(byte)){
                    mCommand = byte;
                    prepareForCommand();
                    mParseState = ParserState::ExpectParam1;
                }
            }
            result = 0;
            break;
            case ParserState::ExpectStart:
            if(byte == START_SYSEX){
                result = 0;
                mParseState = ParserState::ExpectCommand;
                mCurrentMessage = QJsonObject();
                mParams = QJsonArray();
            }
            else{
                result = -1; // error, expecting Start
            }
            break;
            case ParserState::ExpectCommand:
            //TODO: Verify command
                mCommand = byte;
                prepareForCommand();
                mParseState = ParserState::ExpectParam1;
                result = 0;
            break;
            case ParserState::ExpectParam1:
                switch(mCommand){
                    case SYSTEM_RESET:{
                        mParseState = ParserState::ExpectStartOrCommand;
                        result =1;
                        break;
                    }
                    case CAPABILITY_RESPONSE:
                        if(byte == 0x7f){
                            QJsonArray pins = mCurrentMessage.take("pins").toArray();
                            pins.append(mParams);
                            mParams = QJsonArray();
                            mCurrentMessage.insert("pins", pins);
                            result = 0;
                        }
                        else if(byte == END_SYSEX){
                            mParseState = ParserState::ExpectStartOrCommand;
                            result  = 1; // finished parsing capabilities
                        }
                        else{
                            result  = 0;
                            mParams.append(byte);
                            mParseState = ParserState::ExpectParam2;
                        }
                        break;
                    case AM2032_READ:
                        result  = 0; // grab pin or humidity
                        mParams.append(byte);
                        mParseState = ParserState::ExpectParam2;
                        break;
                    case STATUS_QUERY:
                        result  = 0; // grab pin or humidity
                        mParams.append(byte);
                        mParseState = ParserState::ExpectParam2;
                        break;
                    case SET_RELAY:
                        result = 0;
                        mParams.append(byte);
                        mParseState = ParserState::ExpectParam2;
                        break;
                    case SYSTEM_HALT: {
                        result = 1;
                        mParams.append(byte);
                        mParseState = ParserState::ExpectStartOrCommand;
                        break;
                    }
                    case CMD_PROTOCOL_VERSION:
                        result  = 0; // grab protocol version
                        mParams.append(byte);
                        mParseState = ParserState::ExpectParam2;
                        break;
                    case QUERY_FIRMWARE:
                        if(byte == END_SYSEX){
                            parseParams();
                            mParseState = ParserState::ExpectStartOrCommand;
                            result = 1;
                        }
                        else{
                            result  = 0;
                            mParams.append(byte);
                            mParseState = ParserState::ExpectParam1;
                        }
                        break;
                    default: break;
            }
            break;
        case ParserState::ExpectParam2:
            switch(mCommand){
            case CAPABILITY_RESPONSE:{
                mParams.append(byte);
                result = 0;
                mParseState = ParserState::ExpectParam1;
                break;
            }
            case AM2032_READ:{
                if(byte == END_SYSEX){
                    parseParams();
                    result  = 1; // finished reading
                }
                else{
                    result  = 0; // got the temperature
                    mParams.append(byte);
                    mParseState = ParserState::ExpectParam1;
                }
                break;
            }
            case SET_RELAY:{
                mParams.append(byte); // got enabled
                parseParams();
                result = 1;
                mParseState = ParserState::ExpectStartOrCommand;
                break;
            }
            case STATUS_QUERY:{
                quint8 sensor_value = quint8(mParams.first().toInt());
                if(sensor_value > 0 &&sensor_value <= 8){ // reading DHT sensor
                    if(byte == END_SYSEX){
                        parseParams();
                        result  = 1; // finished reading
                        mParseState = ParserState::ExpectStartOrCommand;
                    }
                    else{
                        result  = 0; // got the temperature
                        mParams.append(byte);
                        mParseState = ParserState::ExpectParam1;
                    }
                } else if(sensor_value > 0 && sensor_value <= 12){
                    mParams.append(byte);
                    parseParams();
                    result  = 1;
                    mParseState = ParserState::ExpectStartOrCommand;
                }
                break;
            }
            case CMD_PROTOCOL_VERSION:{
                mParams.append(byte);
                parseParams();
                mParseState = ParserState::ExpectStartOrCommand;
                result  = 1; // finished reading
                break;
            }
            default:break;
            }
            break;
        default:break;
        }
        return result;
    }

    void prepareForCommand(){
        mCurrentMessage.insert("type",mCommand);
    }

    void clearError(){
        bool error;
        quint8 byte = peekNext(error);
        while(!error && byte != END_SYSEX){
            byte = popNext(error);
        }
        mCurrentMessage = QJsonObject();
        mParams = QJsonArray();
        mParseState = ParserState::ExpectStartOrCommand;
    }

    void parseParams()
    {
        switch(mCommand){
            case AM2032_READ:
            {
                if(mParams.count() < 9){
                    return;
                }
                quint8 pin = (mParams.takeAt(0).toInt() & 0xFF);
                FloatMask mask;
                mask.data[3] = (mParams.takeAt(0).toInt());
                mask.data[2] = (mParams.takeAt(0).toInt());
                mask.data[1] = (mParams.takeAt(0).toInt());
                mask.data[0] = (mParams.takeAt(0).toInt());
                float temp = mask.value;
                mask.data[3] = (mParams.takeAt(0).toInt());
                mask.data[2] = (mParams.takeAt(0).toInt());
                mask.data[1] = (mParams.takeAt(0).toInt());
                mask.data[0] = (mParams.takeAt(0).toInt());
                float humidity = mask.value;
                mCurrentMessage.insert("pin",pin);
                mCurrentMessage.insert("temperature", temp);
                mCurrentMessage.insert("humidity",humidity);
                break;
            }
            case STATUS_QUERY:{
                auto sensor_id = quint8(mParams.takeAt(0).toInt());
                if(sensor_id >= 1 && sensor_id <= 8){ // received dht sensor data
                    if(mParams.count() < 8){
                        return;
                    }

                    FloatMask mask;
                    mask.data[3] = (mParams.takeAt(0).toInt());
                    mask.data[2] = (mParams.takeAt(0).toInt());
                    mask.data[1] = (mParams.takeAt(0).toInt());
                    mask.data[0] = (mParams.takeAt(0).toInt());
                    float temp = mask.value;
                    mask.data[3] = (mParams.takeAt(0).toInt());
                    mask.data[2] = (mParams.takeAt(0).toInt());
                    mask.data[1] = (mParams.takeAt(0).toInt());
                    mask.data[0] = (mParams.takeAt(0).toInt());
                    float humidity = mask.value;
                    mCurrentMessage.insert("sensor",sensor_id);
                    mCurrentMessage.insert("temperature", temp);
                    mCurrentMessage.insert("humidity",humidity);
                } else if(sensor_id > 0 && sensor_id <= 12){ // recieved Relay status
                    if(mParams.count() < 1){
                        return;
                    }
                    bool enabled = (mParams.takeAt(0).toInt(0) > 0);
                    mCurrentMessage.insert("sensor", sensor_id);
                    mCurrentMessage.insert("enabled", enabled);
                }
                break;
            }
            case SET_RELAY:{
                if(mParams.size() < 2){
                    return;
                }
                auto sensor_id = quint8(mParams.takeAt(0).toInt());
                auto enabled = (mParams.takeAt(0).toInt(0) > 0);
                mCurrentMessage.insert("sensor", sensor_id);
                mCurrentMessage.insert("enabled", enabled);
                mCurrentMessage.insert("set", true);
                break;
            }
            case QUERY_FIRMWARE:{
                    // first two bytes are major and minor version
                    quint8 major = (mParams.takeAt(0).toInt() & 0xFF);
                    quint8 minor = (mParams.takeAt(0).toInt() & 0xFF);
                    QString version(QString::number(major));
                    version.append(".");
                    version.append(QString::number(minor));
                    mCurrentMessage.insert("version",version);
                    int count = mParams.count();
                    QString name;
                    for(int index(0); index < count; index+=2){
                        char letter = (mParams.takeAt(0).toInt() & 0xFF);
                        mParams.takeAt(0);
                        name.append(letter);
                    }
                    if(!name.isEmpty()){
                        mCurrentMessage.insert("name",name);
                    }
                    break;
            }
            case CMD_PROTOCOL_VERSION:{
                 quint8 major = (mParams.takeAt(0).toInt() & 0xFF);
                 quint8 minor = (mParams.takeAt(0).toInt() & 0xFF);
                 QString version(QString::number(major));
                 version.append(".");
                 version.append(QString::number(minor));
                 mCurrentMessage.insert("version",version);
                 break;
            }
           default:break;
        }
    }

   /* bool parseCommand(quint8 cmd)
    {
        const quint8 nib = cmd & 0xf0;
        switch(nib) {
        case CMD_ANALOG_IO:
        case CMD_DIGITAL_IO:
            mCurrentCommand = nib;
            currentChannel = cmd & 0x0f;
            mParseState = ParserState::ExpectParam1;
            return false;
        }

        switch(cmd) {
        case CMD_SET_DIGITAL_PIN:
        case CMD_PROTOCOL_VERSION:
            mCurrentCommand = cmd;
            mParseState = ParserState::ExpectParam1;
            break;
        case CMD_SYSEX_START:
            mCurrentCommand = cmd;
            sysexdata.clear();
            mParseState = ParserState::ExpectSysexData;
            break;
        case CAPABILITY_RESPONSE:
            mCurrentCommand = cmd;
            mParseState = ParserState::ExpectParam1;
            sysexdata.clear();
            break;
        case CMD_SYSEX_END:{
                mParseState = ParserState::ExpectNothing;
            }

            return true;
            break;
        default:
            qWarning("Unknown command 0x%x", cmd);
            mParseState = ParserState::ExpectNothing;
        }
        return false;
    }

    QJsonObject parse(const quint8& val)
    {
        if((val & 0x80)) {
            // high bit set: this is a command
            parseCommand(val);
            return QJsonObject();
        }
        //if(ParserState::ExpectFirstSysEnd)
        //{

        //}
        // bit 7 zero: parameter data
        switch(mParseState) {
        case ParserState::ExpectNothing:
            break;
        case ParserState::ExpectParam1:
            params[0] = val;
            mParseState = ParserState::ExpectParam2;
            break;
        case ParserState::ExpectParam2:
            params[1] = val;
            //if(m)
            mParseState = ParserState::ExpectNothing;
            //return true;
            break;
        case ParserState::ExpectSysexData:
            sysexdata.append(val);
            break;
        }
        //return false;
    }


    QByteArray  parseSome(int &type,int &action,QJsonArray & message, int count = 40)
    {
        int mParsedCount(0);
        char val;
        bool available(mByteBuffer.length() > 0);
        while(available && mParsedCount < count){
            //continue
            val = mByteBuffer.at(0);
            //QJsonObject chunk = parse(type,action,val);
            mParsedCount++;
            mByteBuffer.remove(0,1);
            available =mByteBuffer.length() > 0;
        }
        return QByteArray();
    }*/
    QBuffer     mBuffer;
    QByteArray  mData;
    ParserState mParseState;
    quint8  mCommand;
    quint8 mCurrent;
    QJsonArray    mParams;
    QJsonObject mCurrentMessage;
private:
    QFirmataParseContext(const QFirmataParseContext & right){
    Q_UNUSED(right);
    }
    QFirmataParseContext operator =(const QFirmataParseContext & right){
        Q_UNUSED(right);
        return *this;
    }
};

#endif // FIRMATA_H
