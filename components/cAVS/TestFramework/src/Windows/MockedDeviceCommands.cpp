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

#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/IoCtlStructureHelpers.hpp"
#include "cAVS/Windows/ModuleHandler.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

template <typename FirmwareParameterType>
void MockedDeviceCommands::addGetModuleParameterCommand(
    dsp_fw::BaseFwParams parameterTypeId,
    const Buffer &returnedParameterContent,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus)
{
    addGetModuleParameterCommand<FirmwareParameterType>(
        parameterTypeId,
        Buffer(returnedParameterContent.getSize()),
        returnedParameterContent,
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus);
}

template <typename FirmwareParameterType>
void MockedDeviceCommands::addGetModuleParameterCommand(
    dsp_fw::BaseFwParams parameterTypeId,
    const Buffer &expectedParameterContent,
    const Buffer &returnedParameterContent,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus)
{
    /* Expected output buffer*/
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> expectedOutput(
        parameterTypeId, expectedParameterContent.getSize());

    /* Filling expected input buffer */
    TypedBuffer<driver::Intc_App_Cmd_Header> expectedInput;
    expectedInput->FeatureID =
        static_cast<ULONG>(driver::FEATURE_MODULE_PARAMETER_ACCESS);
    expectedInput->ParameterID = 0; /* only one parameter id for this feature */
    expectedInput->DataSize = static_cast<ULONG>(expectedOutput.getBuffer().getSize());

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &expectedInput,
            &expectedOutput.getBuffer());
        return;
    }

    /* Returned output buffer*/
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> returnedOutput(
        parameterTypeId, returnedParameterContent.getSize());

    returnedOutput.getCmdBody().Status = returnedDriverStatus;
    if (NT_SUCCESS(returnedDriverStatus)) {

        /* If the driver returns success, set the firmware status*/
        returnedOutput.getModuleParameterAccess().FwStatus = returnedFirmwareStatus;

        if (returnedFirmwareStatus == dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS) {

            /* Setting returned parameter content if the firmware returns success */
            const uint8_t *src =
                reinterpret_cast<const uint8_t *>(returnedParameterContent.getPtr());
            const size_t size = returnedParameterContent.getSize();
            uint8_t *dest = reinterpret_cast<uint8_t *>(&returnedOutput.getFirmwareParameter());

            std::copy(src, src + size, dest);
        }
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &expectedInput,
        &expectedOutput.getBuffer(), &returnedOutput.getBuffer());
}

void MockedDeviceCommands::addTlvParameterCommand(bool ioctlSuccess,
                                                  NTSTATUS returnedDriverStatus,
                                                  dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                  const std::vector<char> &tlvList,
                                                  dsp_fw::BaseFwParams parameterId)
{
    Buffer expectedOutput(ModuleHandler::cavsTlvBufferSize);
    Buffer returnedOutput(tlvList);

    addGetModuleParameterCommand<char>(
        parameterId,
        expectedOutput,
        returnedOutput,
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetFwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                 const std::vector<char> &fwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           fwConfigTlvList,
                           dsp_fw::FW_CONFIG);
}

void MockedDeviceCommands::addGetHwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                 const std::vector<char> &hwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           hwConfigTlvList,
                           dsp_fw::HW_CONFIG_GET);
}

void MockedDeviceCommands::addGetModuleEntriesCommand(bool ioctlSuccess,
    NTSTATUS returnedDriverStatus, dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    const std::vector<ModuleEntry> &returnedEntries)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /** Filling a ModulesInfo structure with the supplied module entries */
    TypedBuffer<dsp_fw::ModulesInfo> buffer(moduleInfoSize);
    buffer->module_count = static_cast<uint32_t>(returnedEntries.size());
    for (std::size_t i = 0; i < returnedEntries.size(); ++i) {
        buffer->module_info[i] = returnedEntries[i];
    }

    addGetModuleParameterCommand<dsp_fw::ModulesInfo>(dsp_fw::MODULES_INFO_GET,
        buffer, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

/* log parameters methods */

void MockedDeviceCommands::addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::FwLogsState &returnedState)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET, &expected.getBuffer(),
            &expected.getBuffer());
        return;
    }

    /* Result code */
    returned.getTinyCmd().Body.Status = returnedStatus;
    if (NT_SUCCESS(returnedStatus)) {

        /* Setting returned log state content if the driver returns success */
        returned.getFwLogsState() = returnedState;
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer());
}

void MockedDeviceCommands::addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::FwLogsState &expectedState)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    /* Setting expected log state content */
    expected.getFwLogsState() = expectedState;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET, &expected.getBuffer(),
            &expected.getBuffer());
        return;
    }

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    /* Result code */
    returned.getTinyCmd().Body.Status = returnedStatus;

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer());
}

}
}
}
