/**
 * @file test_loop_dev.cpp
 * @author Oguzhan MUTLU (oguzhan.mutlu@daiichi.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-27
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/MemoryLeakDetectorMallocMacros.h"

#include "loop_dev.h"

#include <exception>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>

TEST_GROUP(loopDevTest){

    void setup()
    {
        mock().enable();
    }

    void teardown()
    {
        mock().clear();
    }
};

TEST(loopDevTest, ConstructNewDevLoop)
{
    LoopDev *testLoop;

    try
    {
        testLoop = new LoopDev();
    }
    catch(const std::exception& e)
    {
        FAIL("Loop object could not created!");
    } 

    delete testLoop;   
}

TEST(loopDevTest, NoAvailableLoopDevice)
{
    LoopDev *testLoop[8];
    for(int i = 0; i < 8; i++)
    {
        testLoop[i] = new LoopDev();
    }
    
    LoopDev *chkTestDev;

    CHECK_THROWS(std::runtime_error, chkTestDev = new LoopDev());

    for(int i = 0; i < 8; i++)
    {
        delete testLoop[i];
    }
}

TEST(loopDevTest, ConnectAndDisconnectDevice)
{
    int ret = 0;

    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(0);
    mock().expectOneCall("initConnection").andReturnValue(true);
    mock().expectOneCall("sortThreads");
    mock().expectOneCall("runThreads");

    mock().expectOneCall("setQuit");
    mock().expectOneCall("sendToAll").withIntParameter("sig", 12);
    mock().expectOneCall("joinFromThreads");
    mock().expectOneCall("freeThreads");

    LoopDev *testLoop;

    try
    {
        testLoop = new LoopDev();
    }
    catch(const std::exception& e)
    {
        FAIL("Loop object could not created!");
    }

    std::cout << "Capture loop device: " 
                       << testLoop->getCaptureLoopDevName();
    std::cout << "Playback loop device: " 
                       << testLoop->getPlaybackLoopDevName();

    ret = testLoop->connect("hw:2,0");
    CHECK_EQUAL(0, ret);

    ret = testLoop->isLoopConnected();
    CHECK_EQUAL(true, ret);

    ret = testLoop->isRealConnected("hw:2,0");
    CHECK_EQUAL(true, ret);

    ret = testLoop->disconnect();
    CHECK_EQUAL(0, ret);

    mock().checkExpectations();

    delete testLoop;
}

TEST(loopDevTest, TryConnectToAlreadyConnectedRealDev)
{
    int ret = 0;

    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(0);
    mock().expectOneCall("initConnection").andReturnValue(true);
    mock().expectOneCall("sortThreads");
    mock().expectOneCall("runThreads");

    mock().expectOneCall("setQuit");
    mock().expectOneCall("sendToAll").withIntParameter("sig", 12);
    mock().expectOneCall("joinFromThreads");
    mock().expectOneCall("freeThreads");

    LoopDev testLoop;

    std::cout << "Capture loop device: " 
                       << testLoop.getCaptureLoopDevName();
    std::cout << "Playback loop device: " 
                       << testLoop.getPlaybackLoopDevName();

    ret = testLoop.connect("hw:2,0");
    CHECK_EQUAL(0, ret);

    LoopDev testLoop2;

    ret = testLoop2.connect("hw:2,0");
    CHECK_EQUAL(1, ret);

    ret = testLoop.disconnect();
    CHECK_EQUAL(0, ret);
}

TEST(loopDevTest, TryConnectingAnotherRealDevFromTheSameLoopDev)
{
    int ret = 0;

    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(0);
    mock().expectOneCall("initConnection").andReturnValue(true);
    mock().expectOneCall("sortThreads");
    mock().expectOneCall("runThreads");

    mock().expectOneCall("setQuit");
    mock().expectOneCall("sendToAll").withIntParameter("sig", 12);
    mock().expectOneCall("joinFromThreads");
    mock().expectOneCall("freeThreads");

    LoopDev testLoop;

    ret = testLoop.connect("hw:2,0");
    CHECK_EQUAL(0, ret);

    ret = testLoop.connect("hw:3,0");
    CHECK_EQUAL(2, ret);

    ret = testLoop.disconnect();
    CHECK_EQUAL(0, ret);
}

TEST(loopDevTest, GetDevNames)
{
    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(0);
    mock().expectOneCall("initConnection").andReturnValue(true);
    mock().expectOneCall("sortThreads");
    mock().expectOneCall("runThreads");

    mock().expectOneCall("setQuit");
    mock().expectOneCall("sendToAll").withIntParameter("sig", 12);
    mock().expectOneCall("joinFromThreads");
    mock().expectOneCall("freeThreads");

    std::string tmp;
    bool ret = false;

    LoopDev testLoop;

    ret = testLoop.getRealDevName(tmp);

    CHECK_EQUAL(false, ret);

    ret = testLoop.connect("hw:2,0");

    CHECK_EQUAL(0, ret);

    ret = testLoop.getRealDevName(tmp);

    CHECK_EQUAL(true, ret);

    CHECK_EQUAL("hw:2,0", tmp);

    ret = testLoop.disconnect();

    CHECK_EQUAL(0, ret);
}

TEST(loopDevTest, TryDisconnectNotConnectedLoopDevice)
{
    LoopDev testLoop;

    int ret;

    ret = testLoop.disconnect();

    CHECK_EQUAL(1, ret);
}

TEST(loopDevTest, isRealConnectedNotConnectedRealDev)
{
    LoopDev testLoop;

    int ret;

    ret = testLoop.isRealConnected("hw:2,0");

    CHECK_EQUAL(false, ret);
}

TEST(loopDevTest, TryConnectAlsaOutputError)
{
    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(-1);
    mock().expectOneCall("snd_strerror");

    int ret = 0;

    LoopDev testLoop;

    ret = testLoop.connect("hw:2,0");

    CHECK_EQUAL(-1, ret); 
}

TEST(loopDevTest, TryConnectInitError)
{
    mock().expectOneCall("clearQuit");
    mock().expectOneCall("snd_output_stdio_attach").andReturnValue(0);
    mock().expectOneCall("initConnection").andReturnValue(false);

    int ret = 0;

    LoopDev testLoop;

    ret = testLoop.connect("hw:2,0");

    CHECK_EQUAL(-1, ret); 
}