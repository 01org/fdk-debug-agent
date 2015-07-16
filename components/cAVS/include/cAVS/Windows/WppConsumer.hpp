/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code ("Material") are owned by Intel Corporation or its suppliers
*   or licensors.  Title to the  Material remains with  Intel Corporation or its
*   suppliers and licensors. The Material contains trade secrets and proprietary
*   and  confidential  information of  Intel or its suppliers and licensors. The
*   Material  is  protected  by  worldwide  copyright  and trade secret laws and
*   treaty  provisions. No part of the Material may be used, copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way without Intel's prior express written permission.
*   No license  under any  patent, copyright, trade secret or other intellectual
*   property right is granted to or conferred upon you by disclosure or delivery
*   of the Materials,  either expressly, by implication, inducement, estoppel or
*   otherwise.  Any  license  under  such  intellectual property  rights must be
*   express and approved by Intel in writing.
*
********************************************************************************
*/
#pragma once

#include "cAVS/Windows/WppCommon.hpp"
#include "cAVS/Windows/WppLogEntryListener.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include <evntrace.h>
#include <string>
#include <inttypes.h>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class consumes wpp log entries
 *
 * The client has to implement the WppLogEntryListener interface in order to receive entries.
 *
 * Log events can be retrieved either from a realtime session or from a log file.
 */
class WppConsumer final
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /*
     * This method collects log entries, and reports them to the supplied listener.
     *
     * If a file name is supplied, logs are retrieved from a file. Otherwise logs are retrieved
     * from a realtime session.
     *
     * About the blocking behaviour:
     * - realtime session case: this method blocks until the realtime session ends.
     * - log file case: this method blocks until the log file is fully read.
     *
     * @throw WppConsumer::Exception */
    static void collectLogEntries(WppLogEntryListener &listener, const std::string &fileName = "");

private:
    /**
     * Used to filter out all messages except the one carrying FW logs.
     *
     * Important note: This identifier is generated by the WPP preprocessor, and may change
     * unexpectedly if the code of the driver is modified.
     *
     * Here is a comment extracted from this driver source:
     * SW\HDAudioOEDrv\HDAudioOEDrv\OELogListener.cpp
     *
     * "WARNING!!! DO NOT MOVE THIS METHOD OR ADD ANY LOGGING CALLS BEFORE OR INSIDE THIS
     * METHOD OR YOU WILL BREAK EXTERNAL TOOLS THAT EXPECT DoTraceMessage ID TO BE EQUAL TO 10.
     *
     * Tools capturing FW logs may (and do!) depend on WPP message id given to
     * DoTraceMessage. This id is a value of a counter that is (seems to be) incremented
     * with each new logging call encountered by WPP while processing a file top to bottom."
     *
     * Therefore as conclusion this identifier is not reliable, another solution should be
     * investigated.
     */
    static const USHORT fwLogEventDescriptorId = 10;

    /**
     * Header that mirrors the parameters supplied by wpp for each log entry.
     *
     * Here is the driver log entry call:
     *
     * VOID
     * OELogListener::DumpFwLogBuffer(
     * _In_ UINT32 _Id,
     * _In_ UINT32 _Consumed,
     * _In_ UINT32 _WritePosition,
     * _In_ UINT8* _Start)
     * {
     *     DoTraceMessage(
     *         OE_FW, "core id: %08x size: %d position: %d %!HEXDUMP! ",
     *         _Id,
     *         _Consumed,
     *         _WritePosition,
     *         LOG_LENSTR(_Consumed, (PCHAR)_Start));
     * }
     *
     * As you can see the supplied parameters to the DoTraceMessage function are:
     * - the core id (uint32)
     * - consumed, which is the buffer size (uint32)
     * - write position, which is also the buffer size (uint32)
     * - a buffer ("HEXDUMP" token), which is serialized by wpp in this way:
     *    - the buffer size (uint16)
     *    - the buffer content
     */
#pragma pack(1)
    struct FwLogEntry
    {
        uint32_t coreId; /* Supplied by the driver */
        uint32_t size; /* Supplied by the driver */
        uint32_t position; /* Supplied by the driver, currently is always equal to the member
                            * "size" */
        uint16_t wppBufferSize; /* Buffer size supplied by wpp */
        uint8_t buffer[1]; /* Buffer content */
    };
#pragma pack()

    /** This class ensures trace handle deletion */
    class SafeTraceHandler final
    {
    public:
        SafeTraceHandler() : mHandle(INVALID_PROCESSTRACE_HANDLE) {}

        ~SafeTraceHandler()
        {
            close();
        }

        TRACEHANDLE &get()
        {
            return mHandle;
        }

        bool isValid()
        {
            return mHandle != INVALID_PROCESSTRACE_HANDLE;
        }

        void close()
        {
            if (isValid()) {
                ULONG status = CloseTrace(mHandle);
                if (status != ERROR_SUCCESS) {
                    /* @todo: use logging */
                    std::cout << "Unable to close trace handle: err=" << status << std::endl;
                }
            }
        }

    private:
        SafeTraceHandler(const SafeTraceHandler&) = delete;
        SafeTraceHandler& operator=(const SafeTraceHandler&) = delete;

        TRACEHANDLE mHandle;
    };

    /** Callback called by wpp when an event is received */
    static VOID WINAPI ProcessWppEvent(PEVENT_RECORD pEvent);
};

}
}
}
