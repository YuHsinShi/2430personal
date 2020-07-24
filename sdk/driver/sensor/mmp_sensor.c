#include <string.h>
#include "mmp_sensor.h"
#include "adv7180.h"
#include "TVP5150AM1.h"
#include "userdefine.h"
#include "NT99141.h"
pthread_mutex_t internal_mutex = PTHREAD_MUTEX_INITIALIZER;

SensorDriver ithSensorCreate(uint8_t *moduleName)
{

#ifdef CFG_SENSOR_ENABLE
    if (strcmp(moduleName, "adv7180.c") == 0)
        return (SensorDriver)ADV7180SensorDriver_Create();

    if (strcmp(moduleName, "TVP5150AM1.c") == 0)
        return (SensorDriver)TVP5150SensorDriver_Create();

    if (strcmp(moduleName, "NT99141.c") == 0)
        return (SensorDriver)NT99141SensorDriver_Create(); 
    
    if (strcmp(moduleName, "userdefine.c") == 0)
        return (SensorDriver)UserSensorDriver_Create();
#endif
    return NULL;
}

void ithSensorInit(SensorDriver self, uint16_t Mode)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->Init(Mode);
    pthread_mutex_unlock(&internal_mutex);
}

void ithSensorDestroy(SensorDriver self)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->Destroy(self);
    pthread_mutex_unlock(&internal_mutex);
}

void ithSensorDeInit(SensorDriver self)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->Terminate();
    pthread_mutex_unlock(&internal_mutex);
}

uint8_t ithSensorIsSignalStable(SensorDriver self, uint16_t Mode)
{
    uint8_t Isstable = 0;
    pthread_mutex_lock(&internal_mutex);
    if (self)
    {
        Isstable = self->vtable->IsSignalStable(Mode);
        pthread_mutex_unlock(&internal_mutex);
        return Isstable;
    }
    else
        pthread_mutex_unlock(&internal_mutex);

    return Isstable;
}

uint16_t ithSensorGetProperty(SensorDriver self, MODULE_GETPROPERTY Property)
{
    uint16_t value = 0;
    pthread_mutex_lock(&internal_mutex);
    if (self)
    {
        uint16_t value = self->vtable->GetProperty(Property);
        pthread_mutex_unlock(&internal_mutex);
        return value;
    }
    else
        pthread_mutex_unlock(&internal_mutex);
}

uint8_t ithSensorGetStatus(SensorDriver self, MODULE_GETSTATUS Status)
{
    uint8_t status = 0;
    pthread_mutex_lock(&internal_mutex);
    if (self)
    {
        status = self->vtable->GetStatus(Status);
        pthread_mutex_unlock(&internal_mutex);
        return status;
    }
    else
        pthread_mutex_unlock(&internal_mutex); 
    
    return status;
}

void ithSensorSetProperty(SensorDriver self, MODULE_SETPROPERTY Property, uint16_t Value)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->SetProperty(Property, Value);
    pthread_mutex_unlock(&internal_mutex);
}

void ithSensorPowerDown(SensorDriver self, uint8_t Enable)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->PowerDown(Enable);
    pthread_mutex_unlock(&internal_mutex);
}

void ithSensorOutputPinTriState(SensorDriver self, uint8_t Flag)
{
    pthread_mutex_lock(&internal_mutex);
    if (self)
        self->vtable->OutputPinTriState(Flag);
    pthread_mutex_unlock(&internal_mutex);
}