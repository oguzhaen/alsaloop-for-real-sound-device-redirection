/**
 * @file loop_dev.cpp
 * @author Oguzhan MUTLU (oguzhan.mutlu@daiichi.com)
 * @brief Bu sınıf Alsa Loopback device ile source yönetimi için gerekli
 *        fonksiyonları içerir.
 * @brief 
 * @version 0.1
 * @date 2019-11-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <exception>
#include <syslog.h>
#include <iostream>

#include <loop_dev.h>

struct loopbackDev
{
	std::string captureDevName;
	std::string playbackDevName;
	bool isUsed;
};

std::vector<std::string> LoopDev::realDevs;

loopbackDev devTable[8] = {
	{.captureDevName = "hw:0,0,0", .playbackDevName = "hw:0,1,0", .isUsed = false},
	{.captureDevName = "hw:0,0,1", .playbackDevName = "hw:0,1,1", .isUsed = false},
	{.captureDevName = "hw:0,0,2", .playbackDevName = "hw:0,1,2", .isUsed = false},
	{.captureDevName = "hw:0,0,3", .playbackDevName = "hw:0,1,3", .isUsed = false},
	{.captureDevName = "hw:0,0,4", .playbackDevName = "hw:0,1,4", .isUsed = false},
	{.captureDevName = "hw:0,0,5", .playbackDevName = "hw:0,1,5", .isUsed = false},
	{.captureDevName = "hw:0,0,6", .playbackDevName = "hw:0,1,6", .isUsed = false},
	{.captureDevName = "hw:0,0,7", .playbackDevName = "hw:0,1,7", .isUsed = false}};

static loopbackDev *getLoopDev()
{
	loopbackDev *ret = NULL;
	for (int i = 0; i < 8; i++)
	{
		if (false == devTable[i].isUsed)
		{
			devTable[i].isUsed = true;
			ret = &devTable[i];
			break;
		}
	}

	return ret;
}

static void freeLoopDev(loopbackDev *dev)
{
	dev->isUsed = false;
}

LoopDev::LoopDev() : isLoopDevConnected(false)
{
	loopDev = getLoopDev();
	if (NULL == loopDev)
	{
		throw std::runtime_error("No loop device has been found!");
	}

	alsaLoop = new AlsaLoop();
}

LoopDev::~LoopDev()
{
	if (NULL != loopDev)
	{
		freeLoopDev(loopDev);
	}

	delete alsaLoop;
}

int LoopDev::connect(const std::string &realDev)
{
	snd_output_t *output;
	int err;
	std::string tmpRealDevName;

	//Loopdev zaten bağlantılı mı ?
	if (true == isLoopDevConnected)
	{
		std::cout << "Loop device is already connected to a real device ";
		return 2;
	}

	//Realdev başka bir nesnede bağlantı kurmuş mu?
	for (int cnt = 0;
		 cnt < realDevs.size();
		 cnt++)
	{
		if (realDevs.at(cnt) == realDev)
		{
			std::cout << "Real device already connected to a loop device. ";
			return 1;
		}
	}

	std::cout << "Connecting Device : " << realDev ;

	alsaLoop->clearQuit();

	err = snd_output_stdio_attach(&output, stdout, 0);
	if (err < 0)
	{
		std::cerr <<  "Output failed: " << snd_strerror(err);
		return -1;
	}

	tmpRealDevName = realDev;
	char *realDevStr = &tmpRealDevName[0];
	char *loopDevStr = &loopDev->captureDevName[0];

	if (false == alsaLoop->initConnection(realDevStr,
											loopDevStr,
											output))
	{
		std::cerr << "Init Failed!";
		return -1;
	}

	alsaLoop->sortThreads(output);

	alsaLoop->runThreads();
	
	realDevs.push_back(realDev);

	connectedRealDev = realDev;
	isLoopDevConnected = true;

	return 0;
}

int LoopDev::disconnect()
{
	if (false == isLoopDevConnected)
	{
		std::cout << "Loop device not connected any device" ;
		return 1;
	}

	std::cout << "Disconnecting device" ;

	//threade kapanma sinyali gönderiliyor
	alsaLoop->setQuit();
	alsaLoop->sendToAll(SIGUSR2);

	//thread kapanana kadar bekleniyor.
	alsaLoop->joinFromThreads();

	std::cout << "Thread Closed" ;

	alsaLoop->freeThreads();

	isLoopDevConnected = false;
	
    for (auto it = realDevs.begin(); it != realDevs.end(); ++it) 
	{
		if (connectedRealDev == *it)
		{
			realDevs.erase(it);
			break;
		}
	}
	realDevs.shrink_to_fit();

	return 0;
}

std::string LoopDev::getCaptureLoopDevName() const
{
	return loopDev->captureDevName;
}

std::string LoopDev::getPlaybackLoopDevName() const
{
	return loopDev->playbackDevName;
}

bool LoopDev::getRealDevName(std::string &realDevName) const
{
	if (false == isLoopDevConnected)
	{
		return false;
	}

	realDevName = connectedRealDev;

	return true;
}

bool LoopDev::isLoopConnected() const
{
	return isLoopDevConnected;
}

bool LoopDev::isRealConnected(const std::string &realDev) const
{
	for (int cnt = 0;
		 cnt < realDevs.size();
		 cnt++)
	{
		if (realDevs.at(cnt) == realDev)
		{
			std::cout
				<< "Device already connected to a loop device. ";
			return true;
		}
	}

	return false;
}
