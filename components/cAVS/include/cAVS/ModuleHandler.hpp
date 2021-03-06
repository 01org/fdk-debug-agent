/*
 * Copyright (c) 2015-2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cAVS/ModuleHandlerImpl.hpp"
#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/GlobalPerfData.hpp"
#include "cAVS/DspFw/GlobalMemoryState.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "DspFw/Common.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include <stdexcept>
#include <vector>
#include <string>

namespace debug_agent
{
namespace cavs
{

/**
 * The size in bytes of the response buffer needed by the FW in order to reply a TLV.
 * The SwAS specifies the output buffer size for TLV shall be 2KB.
 */
static constexpr size_t tlvBufferSize = 2048;

/**
 * Max parameter payload size set to one memory page (4096 bytes)
 */
static constexpr size_t maxParameterPayloadSize = 4 * 1024;

/** This abstract class exposes the FW module API */
class ModuleHandler
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** Constructor
     *
     * Upon construction, the following FW/HW config items are guaranteed to be valid:
     * - FwConfig::fwVersion
     * - FwConfig::modulesCount
     * - FwConfig::maxPplCount
     * - FwConfig::maxModInstCount
     * - HwConfig::gatewayCount
     * - HwConfig::dspCoreCount
     *
     * @param impl The OS-specific object that communicates with the hardware
     */
    ModuleHandler(std::unique_ptr<ModuleHandlerImpl> impl);
    virtual ~ModuleHandler() {}

    /** @return the firmware module entries */
    const std::vector<dsp_fw::ModuleEntry> &getModuleEntries() const noexcept;

    const dsp_fw::ModuleEntry &findModuleEntry(uint16_t moduleId) const;
    const dsp_fw::ModuleEntry &findModuleEntry(const std::string &name) const;

    /** @return the firmware configuration */
    const dsp_fw::FwConfig &getFwConfig() const noexcept;

    /** @return the hardware configuration */
    const dsp_fw::HwConfig &getHwConfig() const noexcept;

    /** @return the pipeline identifier list */
    std::vector<dsp_fw::PipeLineIdType> getPipelineIdList();

    /** @return the properties of one pipeline */
    dsp_fw::PplProps getPipelineProps(dsp_fw::PipeLineIdType pipelineId);

    /** @return the schedulers of one core */
    dsp_fw::SchedulersInfo getSchedulersInfo(dsp_fw::CoreId coreId);

    /** @return the gateways */
    std::vector<dsp_fw::GatewayProps> getGatewaysInfo();

    /** @return the state of the perf measurement service */
    uint32_t getPerfState();
    /** Sets the state of the perf measurement service */
    void setPerfState(uint32_t state);
    /** @return the performance items */
    std::vector<dsp_fw::PerfDataItem> getPerfItems();

    /** @return the global memory state */
    dsp_fw::GlobalMemoryState getGlobalMemoryState();

    /** @return the properties of one module instance */
    dsp_fw::ModuleInstanceProps getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId);

    /** set module parameter */
    void setModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            const util::Buffer &parameterPayload);

    /** @return module parameter */
    util::Buffer getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                    dsp_fw::ParameterId parameterId,
                                    size_t parameterSize = maxParameterPayloadSize);

    /** The base firmware has several module like components in it.
     * To address them, the ParameterId is splited in a type and an instance part.
     * @{
     */

    /** @return the parameter id addressing the requested core parameters. */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      dsp_fw::CoreId coreId)
    {
        return getExtendedParameterId(parameterTypeId, coreId.getValue());
    }

    /** @return the parameter id addressing the requested pipeline parameters. */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      dsp_fw::PipeLineIdType pipelineId)
    {
        return getExtendedParameterId(parameterTypeId, pipelineId.getValue());
    }
    /** @} */

private:
    /** Perform a "config get" command
     *
     * @param[in] moduleId the module type id
     * @param[in] instanceId the module instance id
     * @param[in] parameterId the parameter id
     * @param[in] parameterSize the parameter's size
     *
     * @returns the parameter payload.
     * @throw ModuleHandler::Exception
     */
    util::Buffer configGet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                           size_t parameterSize);

    /** Perform a "config set" command
     *
     * @param[in] moduleId the module type id
     * @param[in] instanceId the module instance id
     * @param[in] parameterId the parameter id
     * @param[in] parameterPayload the parameter payload to set as value
     *
     * @throw ModuleHandler::Exception
     */
    void configSet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                   const util::Buffer &parameterPayload);

    /** Get module parameter value as a template type
     * @tparam FirmwareParameterType The type of the retrieved parameter value
     */
    template <typename FirmwareParameterType>
    void getFwParameterValue(uint16_t moduleId, uint16_t instanceId,
                             dsp_fw::ParameterId moduleParamId, std::size_t fwParameterSize,
                             FirmwareParameterType &result);

    /** Get a tlv list from a module parameter value */
    template <typename TlvResponseHandlerInterface>
    TlvResponseHandlerInterface readTlvParameters(dsp_fw::BaseFwParams parameterId);

    /** @return extended parameter id that contains the targeted module part id */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      uint32_t parameterInstanceId)
    {
        uint32_t parameterTypeIdAsInt = static_cast<uint32_t>(parameterTypeId);
        assert(parameterTypeIdAsInt < (1 << 8));
        assert(parameterInstanceId < (1 << 24));

        return dsp_fw::ParameterId{(parameterTypeIdAsInt & 0xFF) | (parameterInstanceId << 8)};
    }

    ModuleHandler(const ModuleHandler &) = delete;
    ModuleHandler &operator=(const ModuleHandler &) = delete;

    std::unique_ptr<ModuleHandlerImpl> mImpl;

    // caches (they don't change during runtime and as such can be retrieved at startup time)
    dsp_fw::FwConfig mFwConfig;
    dsp_fw::HwConfig mHwConfig;
    void cacheModuleEntries();
    std::vector<dsp_fw::ModuleEntry> mModuleEntries;
};
}
}
