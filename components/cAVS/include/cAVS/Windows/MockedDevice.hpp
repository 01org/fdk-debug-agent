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

#include "cAVS/Windows/Device.hpp"
#include <memory>
#include <vector>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class is a mocked device implementation
 *
 * Before using it, the user has to fill the test vector.
 * Each entry of this test vector is a 5-tuple:
 * - the expected IO control code
 * - the expected input buffer
 * - the expected output buffer (because the output buffer can be used also as input buffer).
 * - the returned output buffer
 * - the returned IO control status
 *
 * These entries can be added using the addIoctlEntry method.
 *
 * Once the test vector is filled, the mocked device can be used as a real device.
 */
class MockedDevice final : public Device
{
public:
    MockedDevice() : mCurrentEntry(0), mFailed(false) {}
    virtual ~MockedDevice() {}

    /** Thrown when the mock fails, for instance when an input value is not the expected one. */
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : logic_error(msg.c_str()) {}
    };

    /** Add an ioctl entry into the test vector.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] ioControlCode the expected io control code
     * @param[in] expectedInput the expected input buffer
     * @param[in] expectedOutput the expected output buffer (because output buffer can be used
     *                           also as input buffer)
     * @param[in] returnedOutput the returned buffer
     * @param[in] success the returned IO control status
     * @throw Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
        const Buffer *expectedOutput, const Buffer *returnedOutput, bool success);

    /** Check that the mocking sequence is successful, i.e.:
     * - no error has occured
     * - the test vector is fully consumed
     *
     * This method should be called when the test session is finished.
     */
    void checkMockingSuccess()
    {
        if (mFailed) {
            throw Exception("Mocked failed: " + mFailureMessage);
        }
        if (mCurrentEntry != mEntries.size()) {
            throw Exception("IoCtl test vector has not been fully consumed.");
        }
    }

    virtual void ioControl(uint32_t ioControlCode, const Buffer *input, Buffer *output) override;

private:
    /** An IO control entry, which is a 5-tuple */
    class IoCtlEntry final
    {
    public:
        /** Supplied buffers are cloned, ownership is not transferred. */
        IoCtlEntry(uint32_t ioControlCode, const Buffer *expectedInputBuffer,
            const Buffer *expectedOutputBuffer, const Buffer *returnedOutputBuffer,
            bool successsful);

        uint32_t getIOControlCode() const
        {
            return mIoControlCode;
        }

        const Buffer *getExpectedInputBuffer() const
        {
            return mExpectedInputBuffer.get();
        }

        const Buffer *getExpectedOutputBuffer() const
        {
            return mExpectedOutputBuffer.get();
        }

        const Buffer *getReturnedOutputBuffer() const
        {
            return mReturnedOutputBuffer.get();
        }

        bool isSuccessful() const
        {
            return mSuccesssful;
        }

    private:
        uint32_t mIoControlCode;
        std::shared_ptr<Buffer> mExpectedInputBuffer;
        std::shared_ptr<Buffer> mExpectedOutputBuffer;
        std::shared_ptr<Buffer> mReturnedOutputBuffer;
        bool mSuccesssful;
    };

    /** Call this method in case of mock failure */
    void failure(const std::string &msg)
    {
        if (!mFailed) {
            mFailed = true;
            mFailureMessage = msg;
        }
        throw Exception("Mock failed: " + msg);
    }

    /** Call this method in case of mock failure. The current test vector entry is printed. */
    void entryFailure(const std::string &msg)
    {
        failure("IOCtl entry #" + std::to_string(mCurrentEntry) + ": " + msg);
    }

    void checkNonFailure()
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }

    void checkInputBuffer(const Buffer *candidateInputBuffer, const Buffer *expectedInputBuffer);
    void checkAndSetOutputBuffer(Buffer *candidateOutputBuffer, const Buffer *expectedOutputBuffer,
        const Buffer *returnedOutputBuffer);

    using EntryCollection = std::vector<IoCtlEntry>;

    EntryCollection mEntries;
    std::size_t mCurrentEntry;
    bool mFailed;
    std::string mFailureMessage;
};

}
}
}