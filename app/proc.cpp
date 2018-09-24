
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stropts.h>
#include <termios.h>
#include "proc.h"

extern bool     __alive;

/****************************************************************************************
*/
proc::proc()
{
    _subscribed = false;
}

/****************************************************************************************
 * add your console hciconfig preambul to setup hci before BTLE is starting
*/
bool proc::initHciDevice(int devid, const char* devn)
{
    char name[128];
    ::sprintf(name,"btmgmt -i %d power off 1>&2", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d le on 1>&2", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d connectable on 1>&2", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d advertising on 1>&2", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d bredr off 1>&2", devid);
    system(name);
    ::sprintf(name,"btmgmt -i %d power on 1>&2", devid);
    system(name);
    
    return true;
}

/****************************************************************************************
*/
bool proc::onSpin(IServer* ps, uint16_t notyUuid)
{
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
    std::string     ret_hex;
    const uint8_t*  value = pc->get_value();
    char            by[4];
    int             i=0;

    for(;i<pc->get_length();i++)
    {
        ::sprintf(by,"%02X:",value[i]);
        ret_hex.append(by);
        ::sprintf(by,"%c",value[i]);
        ret.append(by);
    }
    TRACE("Remote data:" << ret);
    if(pc->get_16uid() == UID_KEY)
    {
        if (pc->get_length() == 1 && value[0] == 0) {
            TRACE("stoppppppppppppppppppppppp");
            __alive = false;
        }
        else {
            std::cout << ret;
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
void proc::_send_value(IHandler* pc)
{
    uint16_t uid = pc->get_16uid();
    switch(uid)
    {
        case  UID_KEY:
            {
                uint8_t gp = 1;
                GattRw(pc).write(gp);
            }
            break;
        default:
            break;
    }
}


