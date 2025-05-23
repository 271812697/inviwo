/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/tfprimitive.h>               // for TFPrimitive
#include <inviwo/core/datastructures/tfprimitiveset.h>            // for TFPrimitiveSetObserver
#include <inviwo/core/processors/processor.h>                     // for Processor, Processor::N...
#include <inviwo/core/properties/transferfunctionproperty.h>      // for TFPropertyObserver, Tra...
#include <inviwo/core/util/glmvec.h>                              // for dvec2
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>  // for PropertyEditorWidgetQt

#include <functional>  // for function
#include <memory>      // for unique_ptr, shared_ptr
#include <string>      // for string
#include <vector>      // for vector

#include <QSize>  // for QSize

class QColorDialog;
class QComboBox;
class QLabel;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class ColorWheel;
class IsoTFProperty;
class IsoValueProperty;
class Property;
class RangeSliderQt;
class TFColorEdit;
class TFEditor;
class TFEditorView;
class TFLineEdit;
class TFSelectionWatcher;
class TFPropertyConcept;

class IVW_MODULE_QTWIDGETS_API TFPropertyDialog : public PropertyEditorWidgetQt,
                                                  public TFPrimitiveSetObserver,
                                                  public TFPropertyObserver {
public:
    explicit TFPropertyDialog(TransferFunctionProperty* tfProperty);
    explicit TFPropertyDialog(IsoValueProperty* isoProperty);
    explicit TFPropertyDialog(IsoTFProperty* isotfProperty);
    virtual ~TFPropertyDialog();

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    void updateFromProperty();
    TFEditorView* getEditorView() const;

    virtual Property* getProperty() const override;

protected:
    virtual void onTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p) override;
    virtual void onTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type) override;
    virtual void onTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask) override;
    void onTFTypeChangedInternal();

    virtual void onZoomHChange(const dvec2& zoomH) override;
    virtual void onZoomVChange(const dvec2& zoomV) override;
    virtual void onHistogramModeChange(HistogramMode mode) override;

    virtual void setReadOnly(bool readonly) override;

    void changeVerticalZoom(int zoomMin, int zoomMax);
    void changeHorizontalZoom(int zoomMin, int zoomMax);

    virtual void resizeEvent(QResizeEvent*) override;
    virtual void showEvent(QShowEvent*) override;

    void updateTitleFromProperty();
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;

private:
    explicit TFPropertyDialog(std::unique_ptr<TFPropertyConcept> model);

    void updateTFPreview();

    static constexpr int sliderRange_ = 1024;
    static constexpr int verticalSliderRange_ = 1000;

    QLabel* preview_;  ///< View that contains the scene for the painted transfer function

    std::unique_ptr<TFPropertyConcept> concept_;
    std::unique_ptr<ColorWheel> colorWheel_;
    std::unique_ptr<QColorDialog> colorDialog_;
    std::unique_ptr<TFEditor> editor_;  //!< inherited from QGraphicsScene
    std::unique_ptr<TFSelectionWatcher> tfSelectionWatcher_;
    TFEditorView* view_;  //!< View that contains the editor

    QComboBox* chkShowHistogram_;
    QComboBox* pointMoveMode_;

    QLabel* domainMin_;
    QLabel* domainMax_;

    // widgets for directly editing the currently selected TF primitives
    TFLineEdit* primitivePos_;
    TFLineEdit* primitiveAlpha_;
    TFColorEdit* primitiveColor_;

    RangeSliderQt* zoomVSlider_;
    RangeSliderQt* zoomHSlider_;

    bool ongoingUpdate_ = false;
    Processor::NameDispatcherHandle onNameChange_;

    DispatcherHandle<void()> dataChangeHandle_;
};

}  // namespace inviwo
