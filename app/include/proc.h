

#ifndef PROC
#define PROC

#include <libbunget.h>
#include "crypto.h"


/****************************************************************************************
*/
#define UID_GPIO    0x3400
#define UID_TIME    0x3401
#define UID_TEMP    0x3403

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
    void        _prepare_gpio17();
    const char* _get_time();
    float       _get_temp();
    const char* _get_temp_s();
    uint8_t     _get_gpio();
    void        _send_value(IHandler* pc);

public:
    char        _some[20];
    bool        _subscribed;
    IHandler*   LedChr;       // RW
    IHandler*   TimeChr;      // NIR
    IHandler*   Temp1Chr;     // NIR
    IHandler*   EchoCht;
private:
    cryptos     _crypt;         // MANDATORY, detached form lib, Use it on your own GNU
};

#endif
