#include <QDebug>
#include "dsmr_p1_parser.h"

DsmrP1Parser::DsmrP1Parser():
    mState(State::Idle)
{
}

int hexValue(char c)
{
    if (isdigit(c))
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return (c - 'a') + 10;
    if (c >= 'A' && c <= 'F')
        return (c - 'A') + 10;
    return -1;
}

DsmrP1State DsmrP1Parser::process(char c, DsmrP1Message &message)
{
    if (mState != State::Crc && mState != State::Idle)
        mCrc.add(c);
    switch (mState)
    {
    case State::Idle:
        if (c == '/')
        {
            mType.clear();
            mCrc.reset();
            mCrc.add(c);
            mState = State::Type;
        }
        break;
    case State::Type:
        if (processLine(c, mType))
            mState = State::EmptyLine;
        break;
    case State::EmptyLine:
        if (processLine(c, mCurrentLine))
        {
            mState = mCurrentLine.isEmpty() ? State::Line : State::Idle;
            mCurrentLine.clear();
            mLines.clear();
        }
        break;
    case State::Line:
        if (mCurrentLine.isEmpty() && c == '!')
        {
            mMsgCrc = 0;
            mState = State::Crc;
            mCurrentLine.clear();;
            break;
        }
        if (processLine(c, mCurrentLine))
        {
            mLines.append(mCurrentLine);
            mCurrentLine.clear();
        }
        break;
    case State::Crc:
        switch (c)
        {
        case '\r':
            break;
        case '\n':
        {
            mState = State::Idle;
            int crc = mCrc.getValue();
            if (mMsgCrc != crc) {
                qDebug() << "CRC error" << QString::number(crc, 16) << QString::number(mMsgCrc, 16);
                return  DsmrP1State::CrcError;
            }
            message.type = mType;
            message.lines = mLines;
            return DsmrP1State::Ok;
        }
        default:
        {
            int v = hexValue(c);
            if (v == -1) {
                mState = State::Idle;
                break;
            }
            mMsgCrc = (mMsgCrc << 4) | v;
            break;
        }
        }
        break;
    }
    return DsmrP1State::Busy;
}

bool DsmrP1Parser::processLine(char c, QString &line)
{
    switch (c)
    {
    case '\r':
        return false;
    case '\n':
        return true;
    default:
        line += c;
        return false;
    }
}
