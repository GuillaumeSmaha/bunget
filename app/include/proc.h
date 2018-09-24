

#ifndef PROC
#define PROC

#include <libbunget.h>
#include "crypto.h"


/****************************************************************************************
*/
#define UID_KEY    0x3400

/****************************************************************************************
*/
class proc : public ISrvProc
{
public:
    proc();
    Icryptos* get_crypto(){return &_crypt;};
    bool initHciDevice(int devid, const char* name);
    void onServicesDiscovered(std::vector<IHandler*>& els);
    void onReadRequest(IHandler* pc);
    int  onSubscribesNotify(IHandler* pc, bool b);
    void onIndicate(IHandler* pc);
    void onWriteRequest(IHandler* pc);
    void onWriteDescriptor(IHandler* pc, IHandler* pd);
    void onAdvertized(bool onoff);
    void onDeviceStatus(bool onoff);
    void onStatus(const HciDev* connected);
    bool onSpin(IServer* ps, uint16_t notyUuid);

private:
    void        _send_value(IHandler* pc);

public:
    bool        _subscribed;
    IHandler*   Key;       // RW
private:
    cryptos     _crypt;         // MANDATORY, detached form lib, Use it on your own GNU
};

#endif
