/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <inviwo/core/network/processornetworkobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMimeData>
#include <warn/pop>

#include <string>

namespace inviwo {

class CompositeProcessor;
class Property;

class SuperPropertyMimeData : public QMimeData {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    SuperPropertyMimeData(std::vector<Property*> someProperties);
    static std::string_view getMimeTag();
    static const SuperPropertyMimeData* toSuperPropertyMimeData(const QMimeData* data);
    std::vector<Property*> properties;
};

class IVW_QTEDITOR_API SubPropertySelectionDialog : public InviwoDockWidget,
                                                    public ProcessorNetworkObserver {
public:
    SubPropertySelectionDialog(CompositeProcessor* processor, QWidget* parent);
    virtual ~SubPropertySelectionDialog() = default;

protected:
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;

    CompositeProcessor* cp_;
};

}  // namespace inviwo
