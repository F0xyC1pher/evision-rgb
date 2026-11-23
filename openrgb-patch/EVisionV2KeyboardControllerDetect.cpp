/*---------------------------------------------------------*\
| EVisionKeyboardV2ControllerDetect.cpp                     |
|                                                           |
|   Detector for EVision V2 keyboards                       |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "EVisionV2KeyboardController.h"
#include "RGBController_EVisionV2Keyboard.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

/*---------------------------------------------------------*\
| EVision vendor ID                                         |
\*---------------------------------------------------------*/
#define EVISION_VID                                 0x320F
#define SPCGEAR_VID                                 0x3299

/*---------------------------------------------------------*\
| EVision Usage Page                                        |
\*---------------------------------------------------------*/
#define EVISION_KEYBOARD_USAGE_PAGE                 0xFF1C

/*---------------------------------------------------------*\
| Keyboard product IDs                                      |
\*---------------------------------------------------------*/
#define ENDORFY_OMNIS_PID                           0x0012
#define BYGG_CSB_ICL01_PID                          0x5041
#define EVISION_5055_PID                            0x5055

static bool evision5055_registered = false;

DetectedControllers DetectEVisionV2Keyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    json                settings    = ResourceManager::get()->GetSettingsManager()->GetSettings("EVision2Settings");
    hid_device*         dev         = hid_open_path(info->path);

    if(dev)
    {
        EVisionV2KeyboardController*     controller     = new EVisionV2KeyboardController(dev, info->path, EVISION_V2_KEYBOARD_LAYOUT, name);
        RGBController_EVisionV2Keyboard* rgb_controller = new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_KEYBOARD);

        detected_controllers.push_back(rgb_controller);

        if(!settings.contains("AdditionalZones") || settings["AdditionalZones"] == true)
        {
            rgb_controller                             = new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_LOGO);

            detected_controllers.push_back(rgb_controller);

            rgb_controller                             = new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_EDGE);

            detected_controllers.push_back(rgb_controller);
        }
    }

    return(detected_controllers);
}

DetectedControllers DetectEndorfyKeyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    json                settings    = ResourceManager::get()->GetSettingsManager()->GetSettings("EndorfySettings");
    hid_device*         dev         = hid_open_path(info->path);

    if(dev)
    {
        EVisionV2KeyboardController*     controller     = new EVisionV2KeyboardController(dev, info->path, ENDORFY_KEYBOARD_LAYOUT, name);
        RGBController_EVisionV2Keyboard* rgb_controller = new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_KEYBOARD);

        detected_controllers.push_back(rgb_controller);

        if(!settings.contains("AdditionalZones") || settings["AdditionalZones"] == true)
        {
            rgb_controller                              = new RGBController_EVisionV2Keyboard(controller, ENDORFY_KEYBOARD_PART_EDGE);

            detected_controllers.push_back(rgb_controller);
        }
    }

    return(detected_controllers);
}

DetectedControllers DetectEvision5055Keyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    /* One physical interface can expose several hidraw nodes.  Register once. */
    if(evision5055_registered || info->interface_number != 1)
    {
        return detected_controllers;
    }

    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        EVisionV2KeyboardController* controller = new EVisionV2KeyboardController(dev, info->path, EVISION_5055_KEYBOARD_LAYOUT, name);
        detected_controllers.push_back(new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_KEYBOARD));
        evision5055_registered = true;
    }

    return detected_controllers;
}

DetectedControllers DetectEvision5055Wrapped(hidapi_wrapper wrapper, hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(evision5055_registered || info->interface_number != 1)
    {
        return detected_controllers;
    }

    hid_device* dev = wrapper.hid_open_path(info->path);

    if(dev)
    {
        EVisionV2KeyboardController* controller = new EVisionV2KeyboardController(wrapper, dev, info->path, EVISION_5055_KEYBOARD_LAYOUT, name);
        detected_controllers.push_back(new RGBController_EVisionV2Keyboard(controller, EVISION_V2_KEYBOARD_PART_KEYBOARD));
        evision5055_registered = true;
    }

    return detected_controllers;
}

/*---------------------------------------------------------*\
| Keyboards                                                 |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR("EVision RGB Keyboard 320F:5055", DetectEvision5055Keyboards, EVISION_VID, EVISION_5055_PID);
REGISTER_HID_WRAPPED_DETECTOR_I("EVision RGB Keyboard 320F:5055 (libusb)", DetectEvision5055Wrapped, EVISION_VID, EVISION_5055_PID, 1);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:5041",  DetectEVisionV2Keyboards,   EVISION_VID,    BYGG_CSB_ICL01_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Endorfy Omnis",               DetectEndorfyKeyboards,     SPCGEAR_VID,    ENDORFY_OMNIS_PID,  1, EVISION_KEYBOARD_USAGE_PAGE);
