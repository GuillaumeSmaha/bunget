/**
    Copyright:  zirexix 2016-2017

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
    This program should run as root.

    Every time a characterisitc/service is chnaged,
    turn off and on on mobile the BT to clear the cached LE's.

    http://plugable.com/2014/06/23/plugable-usb-bluetooth-adapter-solving-hfphsp-profile-issues-on-linux
    Newer Kernel Versions (3.16 and later)
    wget https://s3.amazonaws.com/plugable/bin/fw-0a5c_21e8.hcd
    sudo mkdir /lib/firmware/brcm
    sudo mv fw-0a5c_21e8.hcd /lib/firmware/brcm/BCM20702A0-0a5c-21e8.hcd

 *              THIS IS A DEMO FOR LIBBUNGET
 *
    This demo adds 1 service 0x123F with 3 characteristis.
        0x3400  control a GPIO pin, we connect a LED, on GPIO 17 '/sys/class/gpio/gpio17/value'
        0x3401
        0x3402


   QT CREATOR APPDEMO
Go to Tools-> Options-> Environment
In the Tab General under **System** Group there is a
Terminal Option.
The default value is set to/usr/bin/xterm -e.
Replace it with /usr/bin/xterm -e sudo or
/usr/bin/gnome-terminal -x sudo.
Press Apply and OK Buttons.
Under Mode Selector click on Projects,
select Run Option. Under Run Group Box
select Run in Terminal


/etc/sudoers using sudo visudo <- /usr/bin/gnome-terminal


*/

#include <unistd.h>
#include <libbunget.h>
#include "args.h"
#include "proc.h"

using namespace std;

bool __alive = true;


int main(int argc, char **argv)
{
    char hname[128] = {0};
    gethostname(hname, sizeof(hname)-sizeof(char));

    args::ArgumentParser parser(LIBBUNGET_VERSION_STRING);
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<int> device(parser, "device", "Device", {'d', "device"}, args::Options::Required);
    args::ValueFlag<int> tweakDelay(parser, "tweak-delay", "Tweak delay", {'w', "tweak-delay"}, 0);
    args::ValueFlag<int> timeAdvetising(parser, "time-advertasing", "Time advertising", {'t', "time-advertasing"}, 512);
    args::ValueFlag<::string> deviceName(parser, "name", "Device name", {'n', "name"}, std::string(hname));
    args::ValueFlag<::string> serviceName(parser, "service-name", "Service name", {'s', "service"}, std::string(hname));

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    TRACE("devcname = " << args::get(deviceName));
    BtConfig config;    
    config._name = args::get(deviceName);
    config._name_short = args::get(deviceName);
    config._tx_power = 0x04;
    config.set_apparence(APPEARANCE_GENERIC_EYE_GLASSES);
    config.set_periphical_pref_conn(BtConfig::S_PERIPHERAL_PREF_CONN::BALANCED);
    config.set_class_of_device(COD_SERVICE_INFORMATION, COD_MAJOR_MISCELLANEOUS, 0);

    BtCtx* ctx = BtCtx::instance();                // BT context
    proc   procedure;                              // this procedure

    try{
        IServer* BS =  ctx->new_server(&procedure, args::get(device), &config, args::get(tweakDelay), true, true);
// #if 0   // not tested !!!
        // BS->set_name("advname"); // this is the bt name.
        //99999999-9999-9999-9999-999999999999
        // BS->adv_beacon("11111111-1111-1111-1111-111111111111", 1, 10, -10, 0x004C, (const uint8_t*)"todo", 7);
// #endif // 0

        IService*   ps = BS->add_service(0x123F, args::get(serviceName).c_str());

        procedure.LedChr = ps->add_charact(UID_GPIO,PROPERTY_WRITE|PROPERTY_INDICATE,
                                 0,
                                 FORMAT_RAW, 1); // 1 / 0

        //procedure.TimeChr = ps->add_charact(UID_TIME, PROPERTY_READ|PROPERTY_NOTIFY,
        procedure.TimeChr = ps->add_charact(UID_TIME, PROPERTY_READ,
                                 0,
                                 FORMAT_RAW, 20); // we send it as string

        //procedure.Temp1Chr = ps->add_charact(UID_TEMP, PROPERTY_NOTIFY|PROPERTY_INDICATE,
        procedure.Temp1Chr = ps->add_charact(UID_TEMP, PROPERTY_READ,
                                  0,
                                  FORMAT_FLOAT, FORMAT_FLOAT_LEN); // we send it as float

        BS->advertise(args::get(timeAdvetising));
        BS->run();
        BS->stop();
    }
    catch(bunget::hexecption& ex)
    {
        ERROR (ex.report());
    }
    return 0;
}
