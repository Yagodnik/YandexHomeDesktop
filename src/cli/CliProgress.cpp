#include "CliProgress.h"

#include <algorithm>

CliProgress::CliProgress(bool enabled, Writer writer, QObject* parent)
    : QObject(parent), enabled_(enabled), writer_(std::move(writer)) {
  timer_.setInterval(100);
  connect(&timer_, &QTimer::timeout, this, &CliProgress::Advance);
}

CliProgress::~CliProgress() {
  Stop();
}

bool CliProgress::EnabledFor(const CliCommand& command, bool terminal) {
  return terminal && !command.json && !command.no_progress;
}

void CliProgress::SetMessage(const QString& message) {
  if (!enabled_) {
    return;
  }
  message_ = message;
  frame_ = 0;
  Render();
  timer_.start();
}

void CliProgress::Stop() {
  timer_.stop();
  message_.clear();
  if (columns_ > 0) {
    writer_(QByteArray("\r") + QByteArray(columns_, ' ') + '\r');
    columns_ = 0;
  }
}

void CliProgress::Advance() {
  if (message_.isEmpty()) {
    return;
  }
  ++frame_;
  Render();
}

void CliProgress::Render() {
  constexpr int travel = 8;
  const int position = frame_ % (2 * travel);
  const int offset = position <= travel ? position : 2 * travel - position;
  const auto bar = QString(offset, ' ') + "===" + QString(travel - offset, ' ');
  const auto line = QString("[%1] %2").arg(bar, message_);
  const int width = static_cast<int>(line.size());
  writer_(("\r" + line + QString(std::max(0, columns_ - width), ' ')).toUtf8());
  columns_ = width;
}
