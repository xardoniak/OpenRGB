/*-----------------------------------------*\
|  RGBController_HyperXAlloyOrigins.cpp     |
|                                           |
|  Generic RGB Interface for HyperX Alloy   |
|  Origins RGB Keyboard                     |
|                                           |
|  Adam Honse (CalcProgrammer1) 7/11/2020   |
\*-----------------------------------------*/

#include "RGBController_HyperXAlloyOrigins.h"

//Include thread libraries for Windows or Linux
#ifdef WIN32
#include <process.h>
#else
#include "pthread.h"
#include "unistd.h"
#endif

//Thread functions have different types in Windows and Linux
#ifdef WIN32
#define THREAD static void
#define THREADRETURN
#else
#define THREAD static void*
#define THREADRETURN return(NULL);
#endif

using namespace std::chrono_literals;

THREAD keepalive_thread(void *param)
{
    RGBController_HyperXAlloyOrigins* controller = static_cast<RGBController_HyperXAlloyOrigins*>(param);
    controller->KeepaliveThread();
    THREADRETURN
}

//0xFFFFFFFF indicates an unused entry in matrix
#define NA  0xFFFFFFFF

static unsigned int matrix_map[6][23] =
    { {   0,  NA,  11,  17,  22,  27,  NA,  33,  38,  43,  49,  NA,  54,  60,  64,  68,  74,  78,  83,  NA,  NA,  NA,  NA },
      {   1,   6,  12,  18,  23,  28,  34,  39,  44,  50,  55,  NA,  61,  65,  69,  NA,  75,  79,  84,  NA,  NA,  NA,  NA },
      {   2,  NA,   7,  13,  19,  24,  NA,  29,  35,  40,  45,  51,  56,  62,  66,  70,  76,  80,  85,  NA,  NA,  NA,  NA },
      {   3,  NA,   8,  14,  20,  25,  NA,  30,  36,  41,  46,  52,  57,  63,  71,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA },
      {   4,  NA,   9,  15,  21,  26,  NA,  31,  NA,  37,  42,  47,  53,  58,  72,  NA,  NA,  81,  NA,  NA,  NA,  NA,  NA },
      {   5,  10,  16,  NA,  NA,  NA,  NA,  32,  NA,  NA,  NA,  NA,  48,  59,  67,  73,  77,  82,  86,  NA,  NA,  NA,  NA } };

static const char* zone_names[] =
{
    "Keyboard",
};

static zone_type zone_types[] =
{
    ZONE_TYPE_MATRIX,
};

static const unsigned int zone_sizes[] =
{
    103,
};

static const char *led_names[] =
{
    "Key: Escape",
    "Key: `",
    "Key: Tab",
    "Key: Caps Lock",
    "Key: Left Shift",
    "Key: Left Control",
    // ?
    "Key: 1",
    "Key: Q",
    "Key: A",
    "Key: Z",
    "Key: Left Windows",
    "Key: F1",
    "Key: 2",
    "Key: W",
    "Key: S",
    "Key: X",
    "Key: Left Alt",
    "Key: F2",
    "Key: 3",
    "Key: E",
    "Key: D",
    "Key: C",
    // ?
    "Key: F3",
    "Key: 4",
    "Key: R",
    "Key: F",
    "Key: V",
    // ?
    "Key: F4",
    "Key: 5",
    "Key: T",
    "Key: G",
    "Key: B",
    "Key: Space",
    "Key: F5",
    "Key: 6",
    "Key: Y",
    "Key: H",
    "Key: N",
    // ?
    "Key: F6",
    "Key: 7",
    "Key: U",
    "Key: J",
    "Key: M",
    //
    "Key: F7",
    "Key: 8",
    "Key: I",
    "Key: K",
    "Key: ,",
    "Key: Right Alt",
    "Key: F8",
    "Key: 9",
    "Key: O",
    "Key: L",
    "Key: .",
    //
    "Key: F9",
    "Key: 0",
    "Key: P",
    "Key: ;",
    "Key: /",
    "Key: Fn",
    "Key: F10",
    "Key: -",
    "Key: [",
    "Key: '",
    //
    //
    "Key: F11",
    "Key: =",
    "Key: ]",
    "Key: Context",
    "Key: F12",
    "Key: Backspace",
    "Key: \\",
    "Key: Enter",
    "Key: Right Shift",
    "Key: Right Control",
    "Key: Print Screen",
    "Key: Insert",
    "Key: Delete",
    //
    //
    "Key: Left Arrow",
    "Key: Scroll Lock",
    "Key: Home",
    "Key: End",
    //
    "Key: Up Arrow",
    "Key: Down Arrow",
    "Key: Pause/Break",
    "Key: Page Up",
    "Key: Page Down",
    //
    //
    "Key: Right Arrow",
    //
    "Key: Num Lock",
    "Key: Number Pad 7",
    "Key: Number Pad 4",
    "Key: Number Pad 0",
    //
    "Key: Number Pad /",
    "Key: Number Pad 8",
    "Key: Number Pad 5",
    "Key: Number Pad 2",
    //
    //
    "Key: Number Pad *",
    "Key: Number Pad 9",
    "Key: Number Pad 6",
    "Key: Number Pad 3",
    "Key: Number Pad .",
    //
    "Key: Number Pad -",
    "Key: Number Pad +",
    "Key: Number Pad Enter", 
};

RGBController_HyperXAlloyOrigins::RGBController_HyperXAlloyOrigins(HyperXAlloyOriginsController* hyperx_ptr)
{
    hyperx = hyperx_ptr;

    name        = "HyperX Alloy Origins Keyboard Device";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "HyperX Alloy Origins Keyboard Device";

    mode Custom;
    Custom.name       = "Custom";
    Custom.value      = 0xFFFF;
    Custom.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Custom.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    SetupZones();

    /*-----------------------------------------------------*\
    | The Corsair Lighting Node Pro requires a packet within|
    | 20 seconds of sending the lighting change in order    |
    | to not revert back into rainbow mode.  Start a thread |
    | to continuously send a keepalive packet every 5s      |
    \*-----------------------------------------------------*/
#ifdef WIN32
    _beginthread(keepalive_thread, 0, this);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, &keepalive_thread, this);
#endif
}

RGBController_HyperXAlloyOrigins::~RGBController_HyperXAlloyOrigins()
{

}

void RGBController_HyperXAlloyOrigins::SetupZones()
{
    /*---------------------------------------------------------*\
    | Set up zones                                              |
    \*---------------------------------------------------------*/
    unsigned int total_led_count = 0;
    for(unsigned int zone_idx = 0; zone_idx < 1; zone_idx++)
    {
        zone new_zone;
        new_zone.name                   = zone_names[zone_idx];
        new_zone.type                   = zone_types[zone_idx];
        new_zone.leds_min               = zone_sizes[zone_idx];
        new_zone.leds_max               = zone_sizes[zone_idx];
        new_zone.leds_count             = zone_sizes[zone_idx];

        if(zone_types[zone_idx] == ZONE_TYPE_MATRIX)
        {
            new_zone.matrix_map         = new matrix_map_type;
            new_zone.matrix_map->height = 6;
            new_zone.matrix_map->width  = 23;
            new_zone.matrix_map->map    = (unsigned int *)&matrix_map;
        }
        else
        {
            new_zone.matrix_map         = NULL;
        }

        zones.push_back(new_zone);

        total_led_count += zone_sizes[zone_idx];
    }

    for(unsigned int led_idx = 0; led_idx < total_led_count; led_idx++)
    {
        led new_led;
        new_led.name = led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_HyperXAlloyOrigins::ResizeZone(int zone, int new_size)
{
    
}
    
void RGBController_HyperXAlloyOrigins::DeviceUpdateLEDs()
{
    hyperx->SetLEDsDirect(colors);
}

void RGBController_HyperXAlloyOrigins::UpdateZoneLEDs(int zone)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXAlloyOrigins::UpdateSingleLED(int led)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXAlloyOrigins::SetCustomMode()
{

}

void RGBController_HyperXAlloyOrigins::DeviceUpdateMode()
{

}

void RGBController_HyperXAlloyOrigins::KeepaliveThread()
{
    while(1)
    {
        if(active_mode == 0)
        {
            if((std::chrono::steady_clock::now() - last_update_time) > std::chrono::milliseconds(50))
            {
                UpdateLEDs();
            }
        }
        std::this_thread::sleep_for(10ms);;
    }
}