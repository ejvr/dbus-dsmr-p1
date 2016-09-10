#ifndef DSMRP1PARSER_H
#define DSMRP1PARSER_H

#include <QStringList>
#include "crc.h"

struct DsmrP1Message
{
    QString type;
    QStringList lines;
};

enum class DsmrP1State
{
    Busy,
    Ok,
    CrcError
};

class DsmrP1Parser
{
public:
    DsmrP1Parser();

    DsmrP1State process(char c, DsmrP1Message &message);

private:
    bool processLine(char c, QString &line);

    enum class State
    {
        Idle,
        Type,
        EmptyLine,
        Line,
        Crc
    };

    State mState;
    QString mType;
    QString mCurrentLine;
    QStringList mLines;
    quint16 mMsgCrc;
    Crc<quint16, 0xA001, 0x0000> mCrc;
};

#endif // DSMRP1PARSER_H
