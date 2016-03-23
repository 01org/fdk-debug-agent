/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/Windows/Probe/ExtractionInputStream.hpp"
#include "Util/RingBufferReader.hpp"
#include "Util/BlockingQueue.hpp"
#include "Util/Exception.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/MemoryStream.hpp"
#include <vector>
#include <memory>
#include <future>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace probe
{

/** Active object that performs probe packets extraction from a ring buffer, and then dispatches
 * them to the matching queue */
class Extractor
{
public:
    using Sample = uint8_t;
    using Exception = util::Exception<Extractor>;
    using FIFO = util::BlockingQueue<util::Buffer>;

    /**
     * @param[in] eventHandle The windows event handle that notifies when the ring buffer
     *                        is filled.
     * @param[in] ringBuffer The extraction ring buffer
     * @param[in] queues the extraction queues that will receive the packetd
     */
    Extractor(EventHandle &eventHandle, util::RingBufferReader &&ringBuffer,
              std::vector<FIFO> &queues)
        : mRingBuffer(std::move(ringBuffer)), mInputStream(eventHandle, mRingBuffer),
          mByteReader(mInputStream), mQueues(queues),
          mExtractionResult{std::async(std::launch::async, &Extractor::extract, this)}
    {
    }

    ~Extractor() { stop(); }

    /** Stop the extractor thread */
    void stop() { mInputStream.close(); }

private:
    void extract()
    {
        try {
            while (true) {

                // reading one packet
                dsp_fw::Packet packet;
                mByteReader.read(packet);

                auto probeId = packet.probePointId;
                if (probeId >= mQueues.size()) {
                    throw Exception("Packet with wrong probe id: " + std::to_string(probeId));
                }

                // Writing the packet to a buffer
                auto buffer = std::make_unique<util::Buffer>();
                util::MemoryOutputStream outputStream(*buffer);
                util::ByteStreamWriter writer(outputStream);
                writer.write(packet);

                // enqueueing the buffer
                if (!mQueues[probeId].add(std::move(buffer))) {
                    std::cerr << "Warning: extraction packet dropped." << std::endl;
                }
            }
        } catch (util::ByteStreamReader::EOSException &) {
            return;
        } catch (std::exception &e) {
            std::string message = "Aborting probe extraction due to: ";
            Exception ex(message + e.what());
            std::cerr << ex.what() << std::endl;
            throw ex;
        }
    }

    util::RingBufferReader mRingBuffer;
    ExtractionInputStream mInputStream;
    util::ByteStreamReader mByteReader;
    std::vector<FIFO> &mQueues;
    std::future<void> mExtractionResult;
};
}
}
}
}