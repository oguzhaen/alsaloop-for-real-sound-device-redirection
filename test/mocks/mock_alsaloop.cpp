/**
 * @file mock_alsaloop.cpp
 * @author Oguzhan MUTLU (oguzhan.mutlu@daiichi.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-27
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CppUTestExt/MockSupport.h"

#include "alsaloop.h"
#include <alsa/asoundlib.h>
#include <signal.h>

AlsaLoop::AlsaLoop()
{

}

AlsaLoop::~AlsaLoop()
{
    
}

void AlsaLoop::signalHandler(int sig)
{
    mock().actualCall("signalHandler");
}
void signalHandlerState(int sig)
{
    mock().actualCall("signalHandlerState");
}
void signalHandlerIgnore(int sig)
{
    mock().actualCall("signalHandlerIgnore");
}
void AlsaLoop::sendToAll(int sig)
{
    mock().actualCall("sendToAll").withIntParameter("sig", sig);
}
int AlsaLoop::sortThreads(snd_output_t * output)
{
    return mock().actualCall("sortThreads").returnIntValueOrDefault(1);
}
void AlsaLoop::threadJob(struct loopbackThread *thread)                                 
{
    mock().actualCall("threadJob");
}
bool AlsaLoop::initConnection(char * realDev, char * loopDev, snd_output_t *output)
{
    return mock().actualCall("initConnection").returnBoolValueOrDefault(true);
}
void AlsaLoop::setQuit()
{
    mock().actualCall("setQuit");
}
void AlsaLoop::clearQuit()
{
    mock().actualCall("clearQuit");
}
void AlsaLoop::freeThreads()
{
    mock().actualCall("freeThreads");
}
void AlsaLoop::joinFromThreads()
{
    mock().actualCall("joinFromThreads");
}
void AlsaLoop::runThreads()
{
    mock().actualCall("runThreads");
}

int snd_output_stdio_attach(snd_output_t **outputp, FILE *fp, int _close)
{
    return mock().actualCall("snd_output_stdio_attach").returnIntValueOrDefault(1);
}

const char *snd_strerror(int errnum)
{
    return mock().actualCall("snd_strerror").returnStringValueOrDefault("A");
}

__sighandler_t signal (int __sig, __sighandler_t __handler)
{
    void * tmpPtr ;
    return (__sighandler_t) mock().actualCall("signal").returnPointerValueOrDefault(tmpPtr);
}

