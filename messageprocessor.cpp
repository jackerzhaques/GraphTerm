#include "messageprocessor.h"

MessageProcessor::MessageProcessor(QObject *parent) : QObject(parent)
{

}

void MessageProcessor::AddBytesToBuffer(QByteArray Data)
{
    this->SerialDataBuffer.append(Data);
    this->ProcessSerialData();
}

void MessageProcessor::ProcessMessage(uint8_t ControlWord, QList<uint32_t> Packets)
{
    switch(ControlWord){
        case COMMAND_ADD_SERIES:
            this->ProcessAddSeriesCommand(Packets);
            break;
        case COMMAND_ADD_SINGLE_DATA:
            this->ProcessAddDataCommand(Packets);
            break;
        case COMMAND_ADD_SERIES_DATA:
            this->ProcessAddSeriesDataCommand(Packets);
            break;
        default:
            //Invalid command, ignore message
            break;
    }
}

void MessageProcessor::ProcessAddSeriesCommand(QList<uint32_t> MessageData)
{
     //Message must be at least 2 bytes
    if(MessageData.size() >= 2){
        int SeriesID = static_cast<int>(MessageData[0]);

        MessageData.removeAt(0);
        QString SeriesName = this->MessageDataToString(MessageData);

        emit(NewSeriesReceived(SeriesID, SeriesName));
    }
    else{
        //The message is invalid
        //Do nothing
    }
}

void MessageProcessor::ProcessAddDataCommand(QList<uint32_t> MessageData)
{
    //The first of MessageData is the ID
    uint8_t ID = static_cast<uint8_t>(MessageData[0]);

    //Trim the ID from the data and convert the rest to floats
    MessageData.removeAt(0);
    QList<float> DataPoints = this->MessageDataToFloats(MessageData);

    //Emit the data
    emit(DataPointsReceived(ID,DataPoints));
}

void MessageProcessor::ProcessAddSeriesDataCommand(QList<uint32_t> MessageData)
{
    //The first of MessageData is the ID
    uint8_t ID = static_cast<uint8_t>(MessageData[0]);

    //Trim the ID from the data and convert the rest to floats
    MessageData.removeAt(0);
    QList<float> DataPoints = this->MessageDataToFloats(MessageData);

    //Emit the data
    emit(DataPointsReceived(ID,DataPoints));
}

//Returns the message with the first and last byte removed
//This trims off the command byte and newline, leaving only data
QByteArray MessageProcessor::TrimMessage(QByteArray Message)
{
    Message.remove(0,1);
    Message.remove(Message.size() - 1, 1);
    return Message;
}

QList<uint32_t> MessageProcessor::GetMessageData(QByteArray Message)
{
    QList<uint32_t> Data;

    //Iterate over every four bytes
    for(int i = 0; i < Message.size(); i += 4){
        uint32_t MessageData = 0;
        MessageData += (static_cast<uint32_t>(Message[i + 0]) << 24) & 0xFF000000;
        MessageData += (static_cast<uint32_t>(Message[i + 1]) << 16) & 0x00FF0000;
        MessageData += (static_cast<uint32_t>(Message[i + 2]) <<  8) & 0x0000FF00;
        MessageData += (static_cast<uint32_t>(Message[i + 3]))       & 0x000000FF;

        Data.append(MessageData);
    }

    return Data;
}

QList<float> MessageProcessor::MessageDataToFloats(QList<uint32_t> Data)
{
    QList<float> FloatData;
    for(int i = 0; i < Data.size(); i++){
        float DataAsFloat = static_cast<float>(*(float*)&Data[i]);

        FloatData.append(DataAsFloat);
    }

    return FloatData;
}

QString MessageProcessor::MessageDataToString(QList<uint32_t> MessageData)
{
    QString string;

    for(int i = 0; i < MessageData.size(); i++){
        string.append(static_cast<char>(MessageData[i]));
    }

    return string;
}

/*
 * Combs through the serial buffer looking for a complete message
 * If a complete message is found, then the data is processed
 *
 * TODO: Optomize this function
 */
void MessageProcessor::ProcessSerialData()
{
    uint8_t ControlWord = 0;                //Read control word
    uint32_t CurrentPacket = 0;             //The data extracted from the message
    uint8_t ByteIndex = 0;        //Used for how far to shift each byte into ProcessedData
    QList<uint32_t> Data;
    bool ControlWordFound = false;
    bool EndOfMessageFound = false;
    for(int i = 0; i < this->SerialDataBuffer.size(); i++){
        uint8_t Byte = static_cast<uint8_t>(this->SerialDataBuffer[i]);
        if(!ControlWordFound){
            //The first data byte must be a control word, if not, delete it
            if(Byte == COMMAND_ADD_SERIES || Byte == COMMAND_ADD_SERIES_DATA || Byte == COMMAND_ADD_SINGLE_DATA){
                ControlWord = Byte;
                ControlWordFound = true;
            }
            else{
                this->SerialDataBuffer.remove(i, 1);
                i--;
            }
        }
        else{
            //if the byte is a newline (0x0A) then end message building
            if(Byte == 0x0A){
                //Remove all the data from the serial buffer up to this byte
                this->SerialDataBuffer.remove(0,i + 1);
                EndOfMessageFound = true;
                break;
            }

            else{
                //Else keep adding to the message
                bool PacketFinished = this->AddByteToPacket(Byte, CurrentPacket, ByteIndex);
                if(PacketFinished){
                    Data.append(CurrentPacket);
                    CurrentPacket = 0;  //Reset the packet variable
                }
            }
        }
    }

    if(EndOfMessageFound){
        this->ProcessMessage(ControlWord, Data);
    }
}

float MessageProcessor::IntToFloat(uint32_t Data)
{
    float result = *(reinterpret_cast<float*>(&Data));
    return result;
}

/*
 * Adds a byte to a uint32_t variable (packet) at the specified index and increments the index
 * Returns true if the index has exceeded the 4th byte
 *
 * This function is used for continuously appending bytes to packets without having
 *  to track the index.
 *
 * Note: the MSB index is 0, and LSB is 3.
 */
bool MessageProcessor::AddByteToPacket(uint8_t Byte, uint32_t &Packet, uint8_t &ByteIndex)
{
    bool PacketFinished = false;

    uint8_t ShiftAmount = (3 - ByteIndex) * 8;
    Packet += static_cast<uint32_t>(Byte) << ShiftAmount;
    ByteIndex++;

    if(ByteIndex > 3){
        ByteIndex = 0;
        PacketFinished = true;
    }

    return PacketFinished;
}
