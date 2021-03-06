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

#include "IfdkObjects/Xml/InstanceDeserializer.hpp"

using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

void InstanceDeserializer::enter(Instance &type, bool isConcrete)
{
    if (isConcrete) {
        pushElement(type);
    }
    type.setTypeName(getStringAttribute(InstanceTraits<Instance>::attributeTypeName));
    type.setInstanceId(getStringAttribute(InstanceTraits<Instance>::attributeInstanceId));
}

void InstanceDeserializer::enter(Component &component, bool isConcrete)
{
    if (isConcrete) {
        pushElement(component);
    }
}

void InstanceDeserializer::enter(Subsystem &subsystem)
{
    pushElement(subsystem);
}

void InstanceDeserializer::enter(System &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(Service &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(EndPoint &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(Ref &ref, bool isConcrete)
{
    assert(!isConcrete);
    ref.setTypeName(getStringAttribute(InstanceTraits<Ref>::attributeTypeName));
    ref.setInstanceId(getStringAttribute(InstanceTraits<Ref>::attributeInstanceId));
}

void InstanceDeserializer::enter(InstanceRef &ref)
{
    pushElement(ref);
}

void InstanceDeserializer::enter(ComponentRef &component)
{
    pushElement(component);
}

void InstanceDeserializer::enter(ServiceRef &service)
{
    pushElement(service);
}

void InstanceDeserializer::enter(EndPointRef &service)
{
    pushElement(service);
}

void InstanceDeserializer::enter(SubsystemRef &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(SystemRef &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(RefCollection &collection, bool isConcrete)
{
    assert(!isConcrete);
    collection.setName(getStringAttribute(InstanceTraits<RefCollection>::attributeName));
}

template <class T>
void InstanceDeserializer::refCollectionCommon(GenericRefCollection<T> &collection)
{
    collection.resize(getChildElementCount());
}

void InstanceDeserializer::enter(InstanceRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(ComponentRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(ServiceRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(EndPointRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(SubsystemRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(Children &chidren)
{
    pushElement(chidren);
    fillPolymorphicVector<RefCollection, InstanceRefCollection, ComponentRefCollection,
                          ServiceRefCollection, EndPointRefCollection, SubsystemRefCollection>(
        chidren.getElements());
}

void InstanceDeserializer::enter(Parents &parents)
{
    pushElement(parents);
    fillPolymorphicVector<Ref, InstanceRef, ComponentRef, SubsystemRef, ServiceRef, EndPointRef>(
        parents.getElements());
}

void InstanceDeserializer::enter(Parameters &, bool isConcrete)
{
    assert(!isConcrete);
}

void InstanceDeserializer::enter(InfoParameters &parameters)
{
    pushElement(parameters);
}

void InstanceDeserializer::enter(ControlParameters &parameters)
{
    pushElement(parameters);
}

void InstanceDeserializer::enter(Connector &connector, bool isConcrete)
{
    assert(!isConcrete);
    connector.setId(getStringAttribute(InstanceTraits<Connector>::attributeId));
    connector.setFormat(getStringAttribute(InstanceTraits<Connector>::attributeFormat));
}

void InstanceDeserializer::enter(Input &connector)
{
    pushElement(connector);
}

void InstanceDeserializer::enter(Output &connector)
{
    pushElement(connector);
}

void InstanceDeserializer::enter(Inputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void InstanceDeserializer::enter(Outputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void InstanceDeserializer::enter(From &instance)
{
    pushElement(instance);
    instance.setTypeName(getStringAttribute(InstanceTraits<From>::attributeTypeName));
    instance.setInstanceId(getStringAttribute(InstanceTraits<From>::attributeInstanceId));
    instance.setOutputId(getStringAttribute(InstanceTraits<From>::attributeOutputId));
}

void InstanceDeserializer::enter(To &instance)
{
    pushElement(instance);
    instance.setTypeName(getStringAttribute(InstanceTraits<To>::attributeTypeName));
    instance.setInstanceId(getStringAttribute(InstanceTraits<To>::attributeInstanceId));
    instance.setInputId(getStringAttribute(InstanceTraits<To>::attributeInputId));
}

void InstanceDeserializer::enter(Link &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(Links &instance)
{
    pushElement(instance);
    instance.resize(getChildElementCount());
}

template <class T>
void InstanceDeserializer::collectionCommon(GenericCollection<T> &collection)
{
    std::size_t childCount = getChildElementCount();
    for (std::size_t i = 0; i < childCount; ++i) {
        collection.add(std::make_shared<T>());
    }
}

void InstanceDeserializer::enter(InstanceCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(ComponentCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(SubsystemCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(ServiceCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(EndPointCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}
}
}
}
