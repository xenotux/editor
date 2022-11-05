/**
 *  xEditor - Editor and tools for creating games
 *  Copyright (C) 2021  Julian Zampiccoli
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef XNG_EDITOR_SCENERENDERWIDGET_HPP
#define XNG_EDITOR_SCENERENDERWIDGET_HPP

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QCoreApplication>

#include "offscreenrenderer.hpp"

class SceneRenderWidget : public QWidget {
Q_OBJECT
public:
    class RenderEvent : public QEvent {
    public:
        float deltaTime;
        QImage image;

        RenderEvent() : QEvent(Type::None) {}
    };

    explicit SceneRenderWidget(QWidget *parent = nullptr) : QWidget(parent),
                                                            ren(30, getSize()) {
        label = new QLabel(this);
        scroll = new QScrollArea(this);

        scroll->setWidget(label);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto *l = new QVBoxLayout();
        l->setMargin(0);
        l->addWidget(scroll);
        setLayout(l);

        ren.setListener([this](float deltaTime, const ImageRGBA &img) {
            auto *event = new RenderEvent();
            event->deltaTime = deltaTime;
            event->image = QImage((const uchar *) img.getData(),
                                  img.getWidth(),
                                  img.getHeight(),
                                  img.getWidth() * sizeof(ColorRGBA),
                                  QImage::Format_RGBA8888);
            QCoreApplication::postEvent(this, event);
        });
    }

    void setScene(std::shared_ptr<EntityScene> scene, std::shared_ptr<std::mutex> mutex) {
        if (scene) {
            ren.setScene(scene, mutex);
        }
    }

protected:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::None) {
            try {
                auto &ev = dynamic_cast<RenderEvent &>(*event);
                label->setPixmap(QPixmap::fromImage(ev.image));
            } catch (...) {}
        }
        return QWidget::event(event);
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        label->setFixedSize(event->size());
        ren.setFrameSize(convert(event->size()));
    }

private:
    Vec2i getSize() {
        return convert(size());
    }

    Vec2i convert(QSize v) {
        return {v.width(), v.height()};
    }

    OffscreenRenderer ren;
    QScrollArea *scroll;
    QLabel *label;
};

#endif //XNG_EDITOR_SCENERENDERWIDGET_HPP
