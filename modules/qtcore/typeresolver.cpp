/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtCore/QObject>
#include <QtCore/QEvent>

#include <global.h>

#include "typeresolver.h"

namespace QtRuby {

void
qeventTypeResolver(QtRuby::Object::Instance * instance)
{
    Smoke::ModuleIndex classId = instance->classId;
    Smoke * smoke = classId.smoke;
    QEvent * qevent = reinterpret_cast<QEvent*>(instance->cast(QtRuby::Global::QEventClassId));
    switch (qevent->type()) {
    case QEvent::Timer:
        instance->classId = smoke->findClass("QTimerEvent");
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        instance->classId = smoke->findClass("QMouseEvent");
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
        instance->classId = smoke->findClass("QKeyEvent");
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        instance->classId = smoke->findClass("QFocusEvent");
        break;
    case QEvent::Enter:
    case QEvent::Leave:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::Paint:
        instance->classId = smoke->findClass("QPaintEvent");
        break;
    case QEvent::Move:
        instance->classId = smoke->findClass("QMoveEvent");
        break;
    case QEvent::Resize:
        instance->classId = smoke->findClass("QResizeEvent");
        break;
    case QEvent::Create:
    case QEvent::Destroy:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::Show:
        instance->classId = smoke->findClass("QShowEvent");
        break;
    case QEvent::Hide:
        instance->classId = smoke->findClass("QHideEvent");
    case QEvent::Close:
        instance->classId = smoke->findClass("QCloseEvent");
        break;
    case QEvent::Quit:
    case QEvent::ParentChange:
    case QEvent::ParentAboutToChange:
    case QEvent::ThreadChange:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::ShowToParent:
    case QEvent::HideToParent:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::Wheel:
        instance->classId = smoke->findClass("QWheelEvent");
        break;
    case QEvent::WindowTitleChange:
    case QEvent::WindowIconChange:
    case QEvent::ApplicationWindowIconChange:
    case QEvent::ApplicationFontChange:
    case QEvent::ApplicationLayoutDirectionChange:
    case QEvent::ApplicationPaletteChange:
    case QEvent::PaletteChange:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::Clipboard:
        instance->classId = smoke->findClass("QClipboardEvent");
        break;
    case QEvent::Speech:
    case QEvent::MetaCall:
    case QEvent::SockAct:
    case QEvent::WinEventAct:
    case QEvent::DeferredDelete:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::DragEnter:
        instance->classId = smoke->findClass("QDragEnterEvent");
        break;
    case QEvent::DragLeave:
        instance->classId = smoke->findClass("QDragLeaveEvent");
        break;
    case QEvent::DragMove:
        instance->classId = smoke->findClass("QDragMoveEvent");
    case QEvent::Drop:
        instance->classId = smoke->findClass("QDropEvent");
        break;
    case QEvent::DragResponse:
        instance->classId = smoke->findClass("QDragResponseEvent");
        break;
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
    case QEvent::ChildPolished:
        instance->classId = smoke->findClass("QChildEvent");
        break;
    case QEvent::ShowWindowRequest:
    case QEvent::PolishRequest:
    case QEvent::Polish:
    case QEvent::LayoutRequest:
    case QEvent::UpdateRequest:
    case QEvent::EmbeddingControl:
    case QEvent::ActivateControl:
    case QEvent::DeactivateControl:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::ContextMenu:
        instance->classId = smoke->findClass("QContextMenuEvent");
        break;
    case QEvent::DynamicPropertyChange:
        instance->classId = smoke->findClass("QDynamicPropertyChangeEvent");
        break;
    case QEvent::InputMethod:
        instance->classId = smoke->findClass("QInputMethodEvent");
        break;
    case QEvent::AccessibilityPrepare:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        instance->classId = smoke->findClass("QTabletEvent");
        break;
    case QEvent::LocaleChange:
    case QEvent::LanguageChange:
    case QEvent::LayoutDirectionChange:
    case QEvent::Style:
    case QEvent::OkRequest:
    case QEvent::HelpRequest:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::IconDrag:
        instance->classId = smoke->findClass("QIconDragEvent");
        break;
    case QEvent::FontChange:
    case QEvent::EnabledChange:
    case QEvent::ActivationChange:
    case QEvent::StyleChange:
    case QEvent::IconTextChange:
    case QEvent::ModifiedChange:
    case QEvent::MouseTrackingChange:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::WindowBlocked:
    case QEvent::WindowUnblocked:
    case QEvent::WindowStateChange:
        instance->classId = smoke->findClass("QWindowStateChangeEvent");
        break;
    case QEvent::ToolTip:
    case QEvent::WhatsThis:
        instance->classId = smoke->findClass("QHelpEvent");
        break;
    case QEvent::StatusTip:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::ActionChanged:
    case QEvent::ActionAdded:
    case QEvent::ActionRemoved:
        instance->classId = smoke->findClass("QActionEvent");
        break;
    case QEvent::FileOpen:
        instance->classId = smoke->findClass("QFileOpenEvent");
        break;
    case QEvent::Shortcut:
        instance->classId = smoke->findClass("QShortcutEvent");
        break;
    case QEvent::WhatsThisClicked:
        instance->classId = smoke->findClass("QWhatsThisClickedEvent");
        break;
    case QEvent::ToolBarChange:
        instance->classId = smoke->findClass("QToolBarChangeEvent");
        break;
    case QEvent::ApplicationActivated:
    case QEvent::ApplicationDeactivated:
    case QEvent::QueryWhatsThis:
    case QEvent::EnterWhatsThisMode:
    case QEvent::LeaveWhatsThisMode:
    case QEvent::ZOrderChange:
        instance->classId = smoke->findClass("QEvent");
        break;
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        instance->classId = smoke->findClass("QHoverEvent");
        break;
    case QEvent::AccessibilityHelp:
    case QEvent::AccessibilityDescription:
        instance->classId = smoke->findClass("QEvent");
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        instance->classId = smoke->findClass("QGraphicsSceneMouseEvent");
        break;
    case QEvent::GraphicsSceneContextMenu:
        instance->classId = smoke->findClass("QGraphicsSceneContextMenuEvent");
        break;
    case QEvent::GraphicsSceneHoverEnter:
    case QEvent::GraphicsSceneHoverMove:
    case QEvent::GraphicsSceneHoverLeave:
        instance->classId = smoke->findClass("QGraphicsSceneHoverEvent");
        break;
    case QEvent::GraphicsSceneHelp:
        instance->classId = smoke->findClass("QGraphicsSceneHelpEvent");
        break;
    case QEvent::GraphicsSceneDragEnter:
    case QEvent::GraphicsSceneDragMove:
    case QEvent::GraphicsSceneDragLeave:
    case QEvent::GraphicsSceneDrop:
        instance->classId = smoke->findClass("QGraphicsSceneDragDropEvent");
        break;
    case QEvent::GraphicsSceneWheel:
        instance->classId = smoke->findClass("QGraphicsSceneWheelEvent");
        break;
    case QEvent::KeyboardLayoutChange:
        instance->classId = smoke->findClass("QEvent");
        break;
    default:
        break;
    }

    instance->value = instance->cast(instance->classId);
    return;
}

void
qobjectTypeResolver(QtRuby::Object::Instance * instance)
{
    Smoke::ModuleIndex classId = instance->classId;
    Smoke * smoke = classId.smoke;
    QObject * qobject = reinterpret_cast<QObject*>(instance->cast(QtRuby::Global::QObjectClassId));
    const QMetaObject * meta = qobject->metaObject();

    while (meta != 0) {
        Smoke::ModuleIndex classId = smoke->findClass(meta->className());
        if (classId != smoke->NullModuleIndex) {
            instance->classId = classId;
            return;
        }

        meta = meta->superClass();
    }

    instance->value = instance->cast(instance->classId);
    return;
}


}
