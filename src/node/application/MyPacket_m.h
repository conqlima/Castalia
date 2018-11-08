//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/application/MyPacket.msg.
//

#ifndef _MYPACKET_M_H_
#define _MYPACKET_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "ApplicationPacket_m.h"
// }}

/**
 * Class generated from <tt>src/node/application/MyPacket.msg:7</tt> by nedtool.
 * <pre>
 * packet MyPacket extends ApplicationPacket
 * {
 *     uint8_t buff[];
 *     int tam_buff;
 * }
 * </pre>
 */
class MyPacket : public ::ApplicationPacket
{
  protected:
    uint8_t *buff_var; // array ptr
    unsigned int buff_arraysize;
    int tam_buff_var;

  private:
    void copy(const MyPacket& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MyPacket&);

  public:
    MyPacket(const char *name=NULL, int kind=0);
    MyPacket(const MyPacket& other);
    virtual ~MyPacket();
    MyPacket& operator=(const MyPacket& other);
    virtual MyPacket *dup() const {return new MyPacket(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual void setBuffArraySize(unsigned int size);
    virtual unsigned int getBuffArraySize() const;
    virtual uint8_t getBuff(unsigned int k) const;
    virtual void setBuff(unsigned int k, uint8_t buff);
    virtual int getTam_buff() const;
    virtual void setTam_buff(int tam_buff);
};

inline void doPacking(cCommBuffer *b, MyPacket& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, MyPacket& obj) {obj.parsimUnpack(b);}


#endif // ifndef _MYPACKET_M_H_

