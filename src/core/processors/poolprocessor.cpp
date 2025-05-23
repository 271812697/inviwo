/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdfuture.h>

#include <fmt/format.h>

namespace inviwo {

void pool::Progress::operator()(float progress) const noexcept {
    state_.setProgress(id_, progress);
}

void pool::Progress::operator()(double progress) const noexcept {
    state_.setProgress(id_, static_cast<float>(progress));
}

void pool::Progress::operator()(size_t i, size_t max) const noexcept {
    state_.setProgress(id_, static_cast<float>(i) / max);
}

void pool::detail::State::setProgress(size_t id, float newProgress) {
    IVW_ASSERT(id < progress.size(), "Invalid job id");

    progress[id] = newProgress;

    if (!progressUpdate.valid() || util::is_future_ready(progressUpdate)) {
        const auto total =
            std::accumulate(progress.begin(), progress.end(), 0.0f) / progress.size();
        progressUpdate = dispatchFront([this, poolProcessor = processor, total]() {
            if (auto p = poolProcessor.lock()) {
                p->progress(this, total);
            }
        });
    }
}

PoolProcessor::PoolProcessor(pool::Options options, const std::string& identifier,
                             const std::string& displayName)
    : Processor(identifier, displayName)
    , options_{options}
    , states_{}
    , notifyRemainingJobsFinish_{[this]() {
        auto running = std::accumulate(states_.begin(), states_.end(), size_t{0},
                                       [](size_t sum, auto& state) { return sum + state->nJobs; });
        notifyObserversFinishBackgroundWork(this, running);
    }}
    , queue_{}
    , delay_{}
    , delayBackgroundJobReset_{nullptr} {

    isReady_.setUpdate([this]() -> ProcessorStatus {
        if (error_) {
            return {ProcessorStatus::Error, error_.value()};
        } else if (allInportsAreReady()) {
            return ProcessorStatus::Ready;
        } else {
            static constexpr std::string_view reason{"Inports are not ready"};
            return {ProcessorStatus::NotReady, reason};
        }
    });
}

PoolProcessor::~PoolProcessor() { stopJobs(); }

void PoolProcessor::stopJobs() {
    for (auto& state : states_) {
        state->stop = true;
    }
}

bool PoolProcessor::hasJobs() { return !states_.empty(); }

void PoolProcessor::submit(Submission& job) {
    job.setupProgress();
    states_.push_back(job.state);
    notifyObserversStartBackgroundWork(this, job.tasks.size());
    for (auto& task : job.tasks) {
        util::getThreadPool(getInviwoApplication()).enqueueRaw(std::move(task));
    }
}

void PoolProcessor::invalidate(InvalidationLevel invalidationLevel, Property* source) {
    error_.reset();
    isReady_.update();

    if (delayInvalidation()) {
        notifyObserversInvalidationBegin(this);
        PropertyOwner::invalidate(invalidationLevel, source);
        notifyObserversInvalidationEnd(this);
    } else {
        Processor::invalidate(invalidationLevel, source);
    }
}

std::string PoolProcessor::handleError() {
    log::error("An error occurred while processing background jobs of {}", getIdentifier());

    std::string message;
    try {
        throw;
    } catch (const Exception& e) {
        message = e.getMessage();
        log::exception(e);
    } catch (const fmt::format_error& e) {
        message = fmt::format("fmt format error: {}\n{}", e.what(), util::fmtHelp.view());
        log::report(LogLevel::Error, message);
    } catch (const std::exception& e) {
        message = e.what();
        log::exception(e);
    } catch (...) {
        message = "Unknown error";
        log::exception();
    }
    for (auto port : getOutports()) {
        port->clear();
    }
    return message;
}

const std::optional<std::string>& PoolProcessor::error() const { return error_; }

void PoolProcessor::newResults() { newResults(getOutports()); }

void PoolProcessor::newResults(const std::vector<Outport*>& outports) {
    notifyObserversInvalidationBegin(this);
    for (auto& outport : outports) {
        outport->invalidate(InvalidationLevel::InvalidOutput);
        outport->setValid();  // Since we don't process this, we need to call setValid on the
                              // outport ourself.
    }
    notifyObserversInvalidationEnd(this);
}

void PoolProcessor::progress(pool::detail::State* state, float progress) {
    if (!states_.empty() && states_.back().get() == state) {
        updateProgress(progress);
    }
}

bool PoolProcessor::removeState(const std::shared_ptr<pool::detail::State>& state) {
    const auto it = std::find(states_.begin(), states_.end(), state);
    IVW_ASSERT(it != states_.end(), "The state should always be found");

    bool isLast = std::next(it) == states_.end();
    states_.erase(it);

    return isLast;
}

Delay& PoolProcessor::getDelay() {
    if (!delay_) {
        delay_.emplace(std::chrono::milliseconds(500),
                       [poolProcessor = std::weak_ptr<PoolProcessor>(
                            std::dynamic_pointer_cast<PoolProcessor>(shared_from_this()))]() {
                           if (auto p = poolProcessor.lock()) {

                               if (p->queuedDispatch() && !p->states_.empty()) {
                                   p->delayBackgroundJobReset_.call();
                                   return;
                               }

                               if (!p->queue_.empty()) {
                                   p->submit(p->queue_.front());
                                   p->queue_.clear();
                               }
                               p->delayBackgroundJobReset_.call();
                           }
                       });
    }
    return delay_.value();
}

}  // namespace inviwo
