################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
#   The source code contained  or  described herein and all documents related to
#   the source code ("Material") are owned by Intel Corporation or its suppliers
#   or licensors.  Title to the  Material remains with  Intel Corporation or its
#   suppliers and licensors. The Material contains trade secrets and proprietary
#   and  confidential  information of  Intel or its suppliers and licensors. The
#   Material  is  protected  by  worldwide  copyright  and trade secret laws and
#   treaty  provisions. No part of the Material may be used, copied, reproduced,
#   modified, published, uploaded, posted, transmitted, distributed or disclosed
#   in any way without Intel's prior express written permission.
#   No license  under any  patent, copyright, trade secret or other intellectual
#   property right is granted to or conferred upon you by disclosure or delivery
#   of the Materials,  either expressly, by implication, inducement, estoppel or
#   otherwise.  Any  license  under  such  intellectual property  rights must be
#   express and approved by Intel in writing.
#
################################################################################

# cmake configuration file of the "Core" component

# Main executable
set(SRCS
    src/Main.cpp
    src/DebugAgent.cpp)

set(INCS
    include/Core/DebugAgent.hpp)

include_directories(include)

add_executable(Core ${SRCS} ${INCS})

# Setting executable name to DebugAgent
set_target_properties(Core PROPERTIES OUTPUT_NAME "DebugAgent")

# Binding with Poco
link_poco(Core)

# Installation step
install(TARGETS Core RUNTIME DESTINATION bin)