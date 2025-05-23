/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwopy/pynetwork.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/vectoridentifierwrapper.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/canvasprocessor.h>

#include <modules/python3/polymorphictypehooks.h>

namespace py = pybind11;

namespace inviwo {

void exposeNetwork(py::module& m) {
    py::classh<PortConnection>(m, "PortConnection")
        .def(py::init<Outport*, Inport*>())
        .def_property_readonly("inport", &PortConnection::getInport,
                               py::return_value_policy::reference)
        .def_property_readonly("outport", &PortConnection::getOutport,
                               py::return_value_policy::reference);

    py::classh<PropertyLink>(m, "PropertyLink")
        .def(py::init<Property*, Property*>(), py::arg("src"), py::arg("dst"))
        .def_property_readonly("source", &PropertyLink::getSource,
                               py::return_value_policy::reference)
        .def_property_readonly("destination", &PropertyLink::getDestination,
                               py::return_value_policy::reference);

    using ProcessorIt = decltype(std::declval<ProcessorNetwork>().processorRange().begin());
    using ProcessorVecWrapper = VectorIdentifierWrapper<ProcessorIt, false>;
    exposeVectorIdentifierWrapper<ProcessorIt, false>(m, "ProcessorVecWrapper");

    py::classh<ProcessorNetwork>(m, "ProcessorNetwork")
        .def_property_readonly("processors",
                               [](ProcessorNetwork& net) {
                                   auto range = net.processorRange();
                                   return ProcessorVecWrapper(range.begin(), range.end());
                               })
        .def("getProcessorByIdentifier", &ProcessorNetwork::getProcessorByIdentifier,
             py::return_value_policy::reference)
        .def(
            "__getattr__",
            [](ProcessorNetwork& po, std::string_view key) {
                if (auto* p = po.getProcessorByIdentifier(key)) {
                    return p;
                } else {
                    throw py::attribute_error{fmt::format(
                        "ProcessorNetwork does not have a processor with identifier: '{}'", key)};
                }
            },
            py::return_value_policy::reference)
        .def("addProcessor",
             [](ProcessorNetwork* pn, std::shared_ptr<Processor> processor) {
                 pn->addProcessor(processor);
             })
        .def("removeProcessor",
             [](ProcessorNetwork* pn, Processor* processor) { pn->removeProcessor(processor); })

        .def_property_readonly("connections", &ProcessorNetwork::getConnections,
                               py::return_value_policy::reference)
        .def(
            "addConnection",
            [&](ProcessorNetwork* on, Outport* sourcePort, Inport* destPort) {
                on->addConnection(sourcePort, destPort);
            },
            py::arg("sourcePort"), py::arg("destPort"))
        .def("addConnection", [&](ProcessorNetwork* on,
                                  PortConnection& connection) { on->addConnection(connection); })
        .def(
            "removeConnection",
            [&](ProcessorNetwork* on, Outport* sourcePort, Inport* destPort) {
                on->removeConnection(sourcePort, destPort);
            },
            py::arg("sourcePort"), py::arg("destPort"))
        .def("removeConnection",
             [&](ProcessorNetwork* on, PortConnection& connection) {
                 on->removeConnection(connection);
             })

        .def(
            "isConnected",
            [&](ProcessorNetwork* on, Outport* sourcePort, Inport* destPort) {
                on->isConnected(sourcePort, destPort);
            },
            py::arg("sourcePort"), py::arg("destPort"))
        .def("isPortInNetwork", &ProcessorNetwork::isPortInNetwork)
        .def_property_readonly("links", &ProcessorNetwork::getLinks,
                               py::return_value_policy::reference)
        .def("addLink",
             [](ProcessorNetwork* pn, Property* src, Property* dst) { pn->addLink(src, dst); })
        .def("addLink", [](ProcessorNetwork* pn, PropertyLink& link) { pn->addLink(link); })
        .def("removeLink",
             [](ProcessorNetwork* pn, Property* src, Property* dst) { pn->removeLink(src, dst); })

        .def("removeLink",
             [](ProcessorNetwork* pn, Property* src, Property* dst) { pn->removeLink(src, dst); })
        .def("isLinked", [](ProcessorNetwork* pn, PropertyLink& link) { pn->isLinked(link); })
        .def("isLinkedBidirectional", &ProcessorNetwork::isLinkedBidirectional)
        .def("getLinksBetweenProcessors", &ProcessorNetwork::getLinksBetweenProcessors,
             py::return_value_policy::reference)
        .def_property_readonly("canvases", &ProcessorNetwork::getProcessorsByType<CanvasProcessor>,
                               py::return_value_policy::reference)
        .def("getProperty", &ProcessorNetwork::getProperty, py::return_value_policy::reference)
        .def("getPort", &ProcessorNetwork::getPort, py::return_value_policy::reference)
        .def("getInport", &ProcessorNetwork::getInport, py::return_value_policy::reference)
        .def("getOutport", &ProcessorNetwork::getOutport, py::return_value_policy::reference)

        .def("getPropertiesLinkedTo", &ProcessorNetwork::getPropertiesLinkedTo,
             py::return_value_policy::reference)
        .def("isPropertyInNetwork", &ProcessorNetwork::isPropertyInNetwork)
        .def_property_readonly("version", &ProcessorNetwork::getVersion)
        .def_property_readonly("empty", &ProcessorNetwork::isEmpty)
        .def_property_readonly("invalidating", &ProcessorNetwork::isInvalidating)
        .def_property_readonly("linking", &ProcessorNetwork::isLinking)
        .def_property_readonly("runningBackgroundJobs", &ProcessorNetwork::runningBackgroundJobs)
        .def("lock", &ProcessorNetwork::lock)
        .def("unlock", &ProcessorNetwork::unlock)
        .def("isLocked", &ProcessorNetwork::islocked)
        .def_property_readonly("locked", &ProcessorNetwork::islocked)
        .def_property_readonly("deserializing", &ProcessorNetwork::isDeserializing)

        .def("clear",
             [&](ProcessorNetwork* pn) { pn->getApplication()->getWorkspaceManager()->clear(); })
        .def("save",
             [](ProcessorNetwork* network, const std::filesystem::path& path) {
                 network->getApplication()->getWorkspaceManager()->save(
                     path, [&](SourceContext) { throw; });  // is this the correct way of
                                                            // re throwing (we just want
                                                            // to pass the exception on to
                                                            // python)
             })
        .def("load",
             [](ProcessorNetwork* network, const std::filesystem::path& path) {
                 network->clear();
                 network->getApplication()->getWorkspaceManager()->load(
                     path, [&](SourceContext) { throw; });  // is this the correct way of re
                                                            // throwing (we just want to pass
                                                            // the exception on to python)
             })

        .def("append", [](ProcessorNetwork* network, const std::filesystem::path& path) {
            return util::appendProcessorNetwork(network, path, network->getApplication());
        });
}
}  // namespace inviwo
