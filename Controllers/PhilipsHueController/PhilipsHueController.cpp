/*---------------------------------------------------------*\
|  Driver for Philips Hue                                   |
|                                                           |
|  Adam Honse (calcprogrammer1@gmail.com), 9/15/2020        |
\*---------------------------------------------------------*/

#include "PhilipsHueController.h"

PhilipsHueController::PhilipsHueController(HueLight& light_ptr):light(light_ptr)
{

}

PhilipsHueController::~PhilipsHueController()
{

}

std::string PhilipsHueController::GetName()
{
    return(light.getModelId());
}

std::string PhilipsHueController::GetVersion()
{
    return(light.getSwVersion());
}

std::string PhilipsHueController::GetManufacturer()
{
    return(light.getManufacturername());
}

std::string PhilipsHueController::GetUniqueID()
{
    return(light.getUId());
}

void PhilipsHueController::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    light.setColorRGB(red, green, blue, 0);
}