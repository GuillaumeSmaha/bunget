
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stropts.h>
#include <termios.h>
#include "proc.h"

/****************************************************************************************
 * intrerrupt the demo in a orthodox way
*/
int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, 0x541B, &bytesWaiting);
    return bytesWaiting;
}


/****************************************************************************************
*/
proc::proc()
{
    _subscribed=false;
    Temp1Chr = 0;
    _prepare_gpio17();
}

/****************************************************************************************
 * add your console hciconfig preambul to setup hci before BTLE is starting
*/
bool proc::initHciDevice(int devid, const char* devn)
{
    char name[128];
    ::sprintf(name,"btmgmt -i %d power off", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d le on", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d connectable on", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d advertising on", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d bredr off", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d power on", devid);
    system(name);
    
    return true;
}

/****************************************************************************************
*/
bool proc::onSpin(IServer* ps, uint16_t notyUuid)
{
    if(_kbhit()){
        if(getchar()=='q')
        return false;
    }


/**
    Control notyUuid frequency from BS->advertise(512<-this value in ms);
    where 512 is the interval in milliseconds when the
    notyUuid flag is passed in for the notification you may send it over.

    If the interval in milliseconds for notification to send
    data, is too small would flood the hci on some dongles, and
    puts dongles in a weird state which requires unpluging and replugging the device.

    for some onboard btle device would rewuire a  full power off

    Tweak the timing from advertise(#) call also as much
    the hci socket to be responsive and the client
    as Android/Ios to be able to eat that data.
    On some Android(s) this got as much as 120 times/ second
    giving a minimum time of 8 ms, but some data were observed to be lost on Modile devices.

    Though on some dongles like noname chinese got in unknown states
    with even 100ms interval.


   In order to have a maximum troughput use read/indicate property and drive the
   read from the mobile device as much the mobile device can suck the data trough it's
   notification / completion event shit. That would go something like.

   read-indicator->  wait callback to complette, push data to main app thread  and issue next read.
   I acheved with this a max 1.4 Kb/second on android and 2k/sec on iOS with a standard HCI 20 bytes payload.


*/
    if(_subscribed)
    {
#ifndef XECHO_BLENO
        if(notyUuid==TimeChr->get_handle())
            _send_value(TimeChr);
        else if(notyUuid==Temp1Chr->get_handle())
            _send_value(Temp1Chr);
#else
//        if(notyUuid==EchoCht->get_handle())
//            _send_value(EchoCht);
#endif
    }
    return true;
}


/****************************************************************************************
*/
void proc::onServicesDiscovered(std::vector<IHandler*>& els)
{
    TRACE("proc event: onServicesDiscovered");
}

/****************************************************************************************
*/
/// remote reads pc characteristics
void proc::onReadRequest(IHandler* pc)
{
    TRACE("proc event:  onReadRequest:" <<  std::hex<< pc->get_16uid() << std::dec);
    _send_value(pc);
}

/****************************************************************************************
*/
int proc::onSubscribesNotify(IHandler* pc, bool b)
{
    TRACE("proc event: onSubscribesNotify:" << std::hex<< pc->get_16uid() << "="<<(int)b<< std::dec);
    _subscribed = b;
    return 0 ;
}

/****************************************************************************************
*/
void proc::onIndicate(IHandler* pc)
{
    TRACE("proc event:  onIndicate:" <<  std::hex<< pc->get_16uid() << std::dec);
    _send_value(pc);
}

/****************************************************************************************
*/
void proc::onWriteRequest(IHandler* pc)
{
    TRACE("proc event:  onWriteRequest:" <<  std::hex<< pc->get_16uid() << std::dec);
    std::string     ret;
    const uint8_t*  value = pc->get_value();
    char            by[4];
    int             i=0;

    for(;i<pc->get_length();i++)
    {
        ::sprintf(by,"%02X:",value[i]);
        ret.append(by);
    }
    TRACE("Remote data:" << ret);
    if(pc->get_16uid() == UID_GPIO)
    {
        if(::access("/sys/class/gpio/gpio17/value",0)==0)
        {
            if(value[0]==0)
                system("echo 0 > /sys/class/gpio/gpio17/value");
            else
                system("echo 1 > /sys/class/gpio/gpio17/value");
        }
    }
}

/****************************************************************************************
*/
//descriptor chnaged of the charact
void proc::onWriteDescriptor(IHandler* pc, IHandler* pd)
{
    TRACE("proc event:  onWriteDescriptor:" << int(*((int*)(pd->get_value()))));
}

/****************************************************************************************
*/
void proc::onAdvertized(bool onoff)
{
    TRACE("proc event:  onAdvertized:" << onoff);
}

/****************************************************************************************
*/
void proc::onDeviceStatus(bool onoff)
{
    TRACE("proc event:  onDeviceStatus:" << onoff);
    if(onoff==false)
    {
        _subscribed = false;
    }
}

/****************************************************************************************
*/
void proc::onStatus(const HciDev* device)
{
    if(device == 0)
    {
        _subscribed = false;
        TRACE("proc event: disconnected");
    }
    else
    {
        TRACE("accepted connection: " << device->_mac <<","<< device->_name);
    }
}

/****************************************************************************************
*/
void proc::_prepare_gpio17()
{
    if(::access("/sys/class/gpio/export/",0)==0)
    {
        system ("chmod 777 /sys/class/gpio/export");
        system ("echo 17 > /sys/class/gpio/export");
        system ("sync");
        if(::access("/sys/class/gpio/gpio17/",0)==0)
            system ("chmod 777 /sys/class/gpio/gpio17/*");
        system ("sync");
    }
}

/****************************************************************************************
*/
const char*  proc::_get_time()
{
    time_t secs = time(0);
    struct tm *local = localtime(&secs);
    sprintf(_some, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
    return _some;
}

/****************************************************************************************
*/
float proc::_get_temp()
{
    float ftamp=0.0;
#ifdef ARM_CC
    if(::access("/opt/vc/bin/vcgencmd",0)==0)
    {
        ::system("/opt/vc/bin/vcgencmd measure_temp > /tmp/bunget");
        std::ifstream ifs("/tmp/bunget");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        temp = temp.substr(5);
        ftamp =::atof(temp.c_str());
    }
#else //fake it
    std::string temp = "temp=32.5";
    temp = temp.substr(5);
    ftamp =::atof(temp.c_str());
    ftamp += rand()%15;
#endif
    return ftamp;
}

/****************************************************************************************
*/
const char* proc::_get_temp_s()
{
#ifdef ARM_CC
    if(::access("/opt/vc/bin/vcgencmd",0)==0)
    {
        ::system("/opt/vc/bin/vcgencmd measure_temp > /tmp/bunget");
        std::ifstream ifs("/tmp/bunget");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        ::strcpy(_some,temp.c_str());
    }
#else //fake it
    static int num = 10;
    ++num;
    ::sprintf(_some,"temp = %d ºC", num);
#endif
    return _some;
}

/****************************************************************************************
*/
uint8_t proc::_get_gpio()
{
    if(::access("/sys/class/gpio/gpio17/value",0)==0)
    {
        std::ifstream ifs("/sys/class/gpio/gpio17/value");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        return uint8_t(::atoi(temp.c_str()));
    }

    return 0;
}

/****************************************************************************************
*/
void proc::_send_value(IHandler* pc)
{
    uint16_t uid = pc->get_16uid();
    switch(uid)
    {
        case  UID_GPIO:
            {
                uint8_t gp = _get_gpio();
                // pc->put_value((uint8_t*)&gp,1);
                GattRw(pc).write(gp);
            }
            break;
        case  UID_TIME:
            {
                const char* t = _get_time();
                pc->put_value((uint8_t*)t,::strlen(t));
            }
            break;
        case  UID_TEMP:
            {
                //float ft = _get_temp();
                //pc->put_value((uint8_t*)&ft,sizeof(float));
                const char* fts = _get_temp_s();
                pc->put_value((uint8_t*)fts,::strlen(fts));
            }
            break;
        case  0xec0e:
            {
                //float ft = _get_temp();
                //pc->put_value((uint8_t*)&ft,sizeof(float));
                //const char* fts = _get_temp_s();
                static int K=0;

                char rands[32];
                ::sprintf(rands,"%d", K++);
                pc->put_value((uint8_t*)rands,::strlen(rands));
            }
        break;
        default:
            break;
    }
}


