/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/progressbar.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_frame.h>
#include <warn/pop>
namespace inviwo {

/**
 * \brief Observes progress bar changes and notifies supplied javascript functions on change.
 */
class IVW_MODULE_WEBBROWSER_API ProgressBarObserverCEF : public ProgressBarObserver {
public:
    ProgressBarObserverCEF(CefRefPtr<CefFrame> frame = nullptr, std::string onProgressChange = "",
                           std::string onVisibleChange = "");
    virtual ~ProgressBarObserverCEF(){};

    /**
     * This method will be called when observed object changes.
     */
    virtual void progressChanged(float progress) override;

    /**
     * This method will be called when observed object changes.
     */
    virtual void progressBarVisibilityChanged(bool visible) override;

    void setOnProgressChange(std::string onChange) { onProgressChange_ = onChange; }
    const std::string& getOnProgressChange() const { return onProgressChange_; }

    void setOnVisibleChange(std::string onChange) { onVisibleChange_ = onChange; }
    const std::string& getOnVisibleChange() const { return onVisibleChange_; }

    /*
     * Set frame containing html item.
     */
    void setFrame(CefRefPtr<CefFrame> frame);

private:
    std::string onProgressChange_;  /// Callback to execute in javascript when progress changes
    std::string onVisibleChange_;   /// Callback to execute in javascript when vibility changes
    CefRefPtr<CefFrame> frame_;     /// Browser frame containing corresponding callbacks
};

}  // namespace inviwo
