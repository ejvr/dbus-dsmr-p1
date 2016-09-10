#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

template<typename T, T P>
class CrcBase
{
public:
    T getValue() const
    {
        return mCrc;
    }

    void add(uint8_t byte)
    {
        int index = (byte ^ mCrc) & 0xFF;
        mCrc >>= 8;
        mCrc ^= mTable[index];
    }

    void add(char byte)
    {
        add(static_cast<uint8_t>(byte));
    }

    template<typename C>
    void add(const C &bytes)
    {
        for (auto b: bytes)
            add(b);
    }

protected:
    CrcBase(T seed):
        mCrc(seed)
    {
        if (!mTableBuilt)
        {
            build_table();
            mTableBuilt = true;
        }
    }

    void reset(T seed)
    {
        mCrc = seed;
    }

private:
    static void build_table()
    {
        for (int v = 0; v<256; ++v)
        {
            T x = v;
            for (int i = 0; i < 8; ++i)
            {
                if ((x & 1) != 0)
                {
                    x >>= 1;
                    x ^= P;
                }
                else
                {
                    x >>= 1;
                }
            }
            mTable[v] = x;
        }
    }

    static T mTable[256];
    static bool mTableBuilt;
    T mCrc;
};

template<typename T, T P> bool CrcBase<T, P>::mTableBuilt = false;
template<typename T, T P> T CrcBase<T, P>::mTable[256];

template<typename T, T P, T S>
class Crc : public CrcBase<T, P>
{
public:
    Crc():
        CrcBase<T, P>(S)
    {}

    void reset()
    {
        CrcBase<T, P>::reset(S);
    }

    using CrcBase<T, P>::getValue;

    template<typename C>
    static T getValue(const C &bytes)
    {
        Crc<T, P, S> crc;
        crc.add(bytes);
        return crc.getValue();
    }
};

#endif // CRC16_H
