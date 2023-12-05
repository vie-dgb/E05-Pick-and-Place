#include "ClickLabel.h"

ClickLabel::ClickLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {

}

ClickLabel::~ClickLabel() {

}

void ClickLabel::mouseMoveEvent(QMouseEvent* event) {
    emit signalEventMouseMove(event);
}

void ClickLabel::mousePressEvent(QMouseEvent* event) {
    emit signalEventMousePress(event);
}

void ClickLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit signalEventMouseRelease(event);
}
