#pragma once

#include <QImage>
#include <QObject>
#include <QUrl>

// Owns a copy of the displayed frame while the native save dialog is open.
class CameraSnapshotModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ Status NOTIFY changed)
  Q_PROPERTY(QUrl suggestedFile READ SuggestedFile NOTIFY changed)
public:
  using QObject::QObject;
  QString Status() const {
    return status_;
  }
  QUrl SuggestedFile() const {
    return suggested_file_;
  }
  Q_INVOKABLE bool Capture(QObject* video_sink);
  Q_INVOKABLE bool Save(const QUrl& destination);
  Q_INVOKABLE void Clear();
signals:
  void changed();

private:
  QImage frame_;
  QString status_;
  QUrl suggested_file_;
};
