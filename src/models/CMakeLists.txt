cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network Gui REQUIRED)

add_library(Models SHARED
        ScenariosModel/ScenariosModel.cpp
        ScenariosModel/ScenariosModel.h
        DevicesModel/DevicesModel.cpp
        DevicesModel/DevicesModel.h
        DevicesModel/DevicesFilterModel.cpp
        DevicesModel/DevicesFilterModel.h
        RoomsModel/RoomsModel.cpp
        RoomsModel/RoomsModel.h
        RoomsModel/RoomsFilterModel.cpp
        RoomsModel/RoomsFilterModel.h
        DeviceModel/CapabilitiesModel.cpp
        DeviceModel/CapabilitiesModel.h
        ColorsModel/ColorsModel.cpp
        ColorsModel/ColorsModel.h
        ColorsModel/ColorModesModel.cpp
        ColorsModel/ColorModesModel.h
        ColorsModel/ColorsFilterModel.cpp
        ColorsModel/ColorsFilterModel.h
        ColorsModel/ColorModesFilterModel.cpp
        ColorsModel/ColorModesFilterModel.h
        HouseholdsModel/HouseholdsModel.cpp
        HouseholdsModel/HouseholdsModel.h
        DeviceModel/DeviceAttribute.h
        DeviceModel/DeviceAttribute.cpp
        DeviceModel/PropertiesModel.cpp
        DeviceModel/PropertiesModel.h
        ModesModel/ModesModel.cpp
        ModesModel/ModesModel.h
        ModesModel/ModesFilterModel.cpp
        ModesModel/ModesFilterModel.h
        DeviceModel/DeviceDataModel.cpp
        DeviceModel/DeviceDataModel.h
        #        DeviceModel/DeviceDataController.cpp
#        DeviceModel/DeviceDataController.h
)

target_link_libraries(Models PRIVATE Qt6::Core Qt6::Network Qt6::Gui YandexHomeApi)
