#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

/*
 * Processes messages sent by the platform, extracts data, and performs commands
 */

/*
 * Message Format
 *
 * CMD d0 d1 d2 d3...dx
 *
 * CMD:         Command code. See command code defines for a list of commands. This is a single byte
 * d0,d1,dx:    Data bytes. Each data point (i.e. d0) is four bytes wide. Byte streaming ends when a newline is received after four bytes.
 *
 * Errors:
 *  Framing errors:
 *      To shorten messages, data is sent as raw bytes rather than ASCII characters(i.e. '0', '1'). Because of this,
 *          messages cannot be framed with ascii characters, like comma separated values. If a user wishes to send the
 *          byte 0x2C, it would be interpreted as a ','.
 *
 *      Framing errors will occur when:
 *          -Data isn't padded to be exactly four bytes. (i.e. you send 0xFF 0xFF 0xFF instead of 0x00 0xFF 0xFF 0xFF)
 *
 *      Prevent framing errors by ensuring your data is padded correctly.
 *
 *      To prevent software from locking up due to framing errors, if the incorrect sequence of bytes is received, the
 *          software will cancel the framing and dump all previous data.
 */

/*
 * Example commands
 *
 * Assume no spaces between bytes
 * 0x## is a number in hex
 * Any character surrounded with '' is an ASCII character.
 *
 *
 *
 * Add Series
 *
 *  Format:
 *      CMD ID NAME NEWLINE
 *
 *      CMD:        The command ID for adding a series.
 *      ID:         The unique ID to be associated with the data series
 *      NAME:       The name to be given to the data series. Does not need to be unique, but recommended
 *      NEWLINE:    An ASCII newline (
 *
 *  Example:
 *      0x01 0x00 x00 0x00 0xFF 0x00 0x00 0x00 'T' 0x00 0x00 0x00 'E' 0x00 0x00 0x00 'S' 0x00 0x00 0x00 'T' 0x0A
 *
 *      Creates a data series with the name "TEST" and ID 255
 *
 *
 *
 * Add Single Data Point
 *
 *  Format:
 *      CMD ID DATA NEWLINE
 *
 *      CMD:    The command ID for adding a single data point
 *      ID:     The unique ID representing the series
 *      DATA:   The value of the data point. NOTE. This is a 32-bit floating point number!
 *
 *  Example:
 *      0x02 0x00 0x00 0x00 0xFF 0xBF 0x00 0x00 0xFF 0x0A
 *
 *      Adds a data point to the series with the ID '255' with the
 *          value -0.500015.
 *
 * Add Series of Data Points
 *
 *  Format:
 *      CMD ID DATA1 DATA2 ...DATAX NEWLINE
 *
 *      CMD:    The command ID for adding a single data point
 *      ID:     The unique ID representing the series
 *      DATAX:  A stream of data, every four bytes being a new data point. NOTE. These bytes are 32-bit floating point numbers!
 *
 *  Example:
 *      0x03 0x00 0x00 0x00 0xFF 0xBF 0x00 0x00 0xFF 0xC0 0x00 0x00 0x00 0xC1 0x00 0x00 0x00 0x0A
 *
 *      Adds to the data series with ID 255 the following data points:
 *          -0.500015
 *          -2
 *          -8
 *
 *
 */

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QDebug>

//Command bytes
#define COMMAND_ADD_SERIES              0x01
#define COMMAND_ADD_SINGLE_DATA         0x02
#define COMMAND_ADD_SERIES_DATA         0x03

class MessageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MessageProcessor(QObject *parent = nullptr);
    void AddBytesToBuffer(QByteArray Data);

signals:
    void NewSeriesReceived(int Series, QString Seriesname);
    void DataPointsReceived(int SeriesID, QList<float> DataPoints);

public slots:
    void ProcessMessage(uint8_t ControlWord, QList<uint32_t> Packets);
    void ProcessAddSeriesCommand(QList<uint32_t> MessageData);
    void ProcessAddDataCommand(QList<uint32_t> MessageData);
    void ProcessAddSeriesDataCommand(QList<uint32_t> MessageData);
    QByteArray TrimMessage(QByteArray Message);
    QList<uint32_t> GetMessageData(QByteArray Message);
    QList<float> MessageDataToFloats(QList<uint32_t> Data);
    QString MessageDataToString(QList<uint32_t> MessageData);

private:
    void ProcessSerialData();
    float IntToFloat(uint32_t Data);
    bool AddByteToPacket(uint8_t Byte, uint32_t &Packet, uint8_t &ByteIndex);
    QByteArray SerialDataBuffer;

};

#endif // MESSAGEPROCESSOR_H
