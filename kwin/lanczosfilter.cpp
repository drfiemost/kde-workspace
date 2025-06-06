/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 by Fredrik Höglund <fredrik@kde.org>
Copyright (C) 2010 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "lanczosfilter.h"
#include "client.h"
#include "deleted.h"
#include "effects.h"
#include "unmanaged.h"
#include "options.h"
#include "workspace.h"

#include <kwinglutils.h>
#include <kwinglplatform.h>

#include <kwineffects.h>
#include <KDE/KGlobalSettings>

#include <qmath.h>
#include <cmath>

namespace KWin
{

LanczosFilter::LanczosFilter(QObject* parent)
    : QObject(parent)
    , m_offscreenTex(0)
    , m_offscreenTarget(0)
    , m_inited(false)
    , m_shader(0)
    , m_uTexUnit(0)
    , m_uOffsets(0)
    , m_uKernel(0)
{
}

LanczosFilter::~LanczosFilter()
{
    delete m_offscreenTarget;
    delete m_offscreenTex;
}

void LanczosFilter::init()
{
    if (m_inited)
        return;
    m_inited = true;
    const bool force = (qstrcmp(qgetenv("KWIN_FORCE_LANCZOS"), "1") == 0);
    if (force) {
        kWarning(1212) << "Lanczos Filter forced on by environment variable";
    }

    if (!force && options->glSmoothScale() != 2)
        return; // disabled by config
    if (!GLRenderTarget::supported())
        return;

    GLPlatform *gl = GLPlatform::instance();
    if (!force) {
        // The lanczos filter is reported to be broken with the Intel driver prior SandyBridge
        if (gl->driver() == Driver_Intel && gl->chipClass() < SandyBridge)
            return;
        // Broken on Intel chips with Mesa 9.1 - BUG 313613
        if (gl->driver() == Driver_Intel && gl->mesaVersion() >= kVersionNumber(9, 1) && gl->mesaVersion() < kVersionNumber(9, 2))
            return;
        // also radeon before R600 has trouble
        if (gl->isRadeon() && gl->chipClass() < R600)
            return;
    }
    m_shader.reset(ShaderManager::instance()->loadFragmentShader(ShaderManager::SimpleShader,
                                                                 gl->glslVersion() >= kVersionNumber(1, 40) ?
                                                                 ":/resources/shaders/1.40/lanczos-fragment.glsl" :
                                                                 ":/resources/shaders/1.10/lanczos-fragment.glsl"));
    if (m_shader->isValid()) {
        ShaderBinder binder(m_shader.data());
        m_uTexUnit    = m_shader->uniformLocation("texUnit");
        m_uKernel     = m_shader->uniformLocation("kernel");
        m_uOffsets    = m_shader->uniformLocation("offsets");
    } else {
        kDebug(1212) << "Shader is not valid";
        m_shader.reset();
    }
}


void LanczosFilter::updateOffscreenSurfaces()
{
    int w = displayWidth();
    int h = displayHeight();
    if (!GLTexture::NPOTTextureSupported()) {
        w = nearestPowerOfTwo(w);
        h = nearestPowerOfTwo(h);
    }
    if (!m_offscreenTex || m_offscreenTex->width() != w || m_offscreenTex->height() != h) {
        if (m_offscreenTex) {
            delete m_offscreenTex;
            delete m_offscreenTarget;
        }
        m_offscreenTex = new GLTexture(w, h);
        m_offscreenTex->setFilter(GL_LINEAR);
        m_offscreenTex->setWrapMode(GL_CLAMP_TO_EDGE);
        m_offscreenTarget = new GLRenderTarget(*m_offscreenTex);
    }
}

static float sinc(float x)
{
    return std::sin(x * M_PI) / (x * M_PI);
}

static float lanczos(float x, float a)
{
    if (qFuzzyCompare(x + 1.0, 1.0))
        return 1.0;

    if (std::abs(x) >= a)
        return 0.0;

    return sinc(x) * sinc(x / a);
}

void LanczosFilter::createKernel(float delta, int *size)
{
    const float a = 2.0;

    // The two outermost samples always fall at points where the lanczos
    // function returns 0, so we'll skip them.
    const int sampleCount = std::clamp(static_cast<int>(std::ceil(delta * a)) * 2 + 1 - 2, 3, 29);
    const int center = sampleCount / 2;
    const int kernelSize = center + 1;
    const float factor = 1.0 / delta;

    QVector<float> values(kernelSize);
    float sum = 0;

    for (int i = 0; i < kernelSize; i++) {
        const float val = lanczos(i * factor, a);
        sum += i > 0 ? val * 2 : val;
        values[i] = val;
    }

    // Normalize the kernel
    for (int i = 0; i < kernelSize; i++) {
        const float val = values[i] / sum;
        m_kernel[i] = QVector4D(val, val, val, val);
    }

    for (int i = kernelSize; i < 16; i++)
        m_kernel[i] = QVector4D();

    *size = kernelSize;
}

void LanczosFilter::createOffsets(int count, float width, Qt::Orientation direction)
{
    for (int i = 0; i < count; i++) {
        m_offsets[i] = (direction == Qt::Horizontal) ?
                       QVector2D(i / width, 0) : QVector2D(0, i / width);
    }
    for (int i = count; i < 16; i++)
        m_offsets[i] = QVector2D();
}

void LanczosFilter::performPaint(EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data)
{
    if ((data.xScale() < 0.9 || data.yScale() < 0.9) &&
            KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
        if (!m_inited)
            init();
        const QRect screenRect = Workspace::self()->clientArea(ScreenArea, w->screen(), w->desktop());
        // window geometry may not be bigger than screen geometry to fit into the FBO
        QRect winGeo(w->expandedGeometry());
        if (m_shader && winGeo.width() <= screenRect.width() && winGeo.height() <= screenRect.height()) {
            winGeo.translate(-w->geometry().topLeft());
            double left = winGeo.left();
            double top = winGeo.top();
            double width = winGeo.right() - left;
            double height = winGeo.bottom() - top;

            int tx = data.xTranslation() + w->x() + left * data.xScale();
            int ty = data.yTranslation() + w->y() + top * data.yScale();
            int tw = width * data.xScale();
            int th = height * data.yScale();
            const QRect textureRect(tx, ty, tw, th);
            const bool hardwareClipping = !(QRegion(textureRect)-region).isEmpty();

            int sw = width;
            int sh = height;

            GLTexture *cachedTexture = static_cast< GLTexture*>(w->data(LanczosCacheRole).value<void*>());
            if (cachedTexture) {
                if (cachedTexture->width() == tw && cachedTexture->height() == th) {
                    cachedTexture->bind();
                    if (hardwareClipping) {
                        glEnable(GL_SCISSOR_TEST);
                    }

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                    const qreal rgb = data.brightness() * data.opacity();
                    const qreal a = data.opacity();

                    ShaderBinder binder(ShaderManager::SimpleShader);
                    GLShader *shader = binder.shader();
                    shader->setUniform(GLShader::Offset, QVector2D(0, 0));
                    shader->setUniform(GLShader::ModulationConstant, QVector4D(rgb, rgb, rgb, a));
                    shader->setUniform(GLShader::Saturation, data.saturation());

                    cachedTexture->render(region, textureRect, hardwareClipping);

                    glDisable(GL_BLEND);
                    if (hardwareClipping) {
                        glDisable(GL_SCISSOR_TEST);
                    }
                    cachedTexture->unbind();
                    m_timer.start(5000, this);
                    return;
                } else {
                    // offscreen texture not matching - delete
                    delete cachedTexture;
                    cachedTexture = 0;
                    w->setData(LanczosCacheRole, QVariant());
                }
            }

            WindowPaintData thumbData = data;
            thumbData.setXScale(1.0);
            thumbData.setYScale(1.0);
            thumbData.setXTranslation(-w->x() - left);
            thumbData.setYTranslation(-w->y() - top);
            thumbData.setBrightness(1.0);
            thumbData.setOpacity(1.0);
            thumbData.setSaturation(1.0);

            // Bind the offscreen FBO and draw the window on it unscaled
            updateOffscreenSurfaces();
            GLRenderTarget::pushRenderTarget(m_offscreenTarget);

            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT);
            w->sceneWindow()->performPaint(mask, infiniteRegion(), thumbData);

            // Create a scratch texture and copy the rendered window into it
            GLTexture tex(sw, sh);
            tex.setFilter(GL_LINEAR);
            tex.setWrapMode(GL_CLAMP_TO_EDGE);
            tex.bind();

            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - sh, sw, sh);

            // Set up the shader for horizontal scaling
            float dx = sw / float(tw);
            int kernelSize;
            createKernel(dx, &kernelSize);
            createOffsets(kernelSize, sw, Qt::Horizontal);

            ShaderManager::instance()->pushShader(m_shader.data());
            setUniforms();

            // Draw the window back into the FBO, this time scaled horizontally
            glClear(GL_COLOR_BUFFER_BIT);
            QVector<float> verts;
            QVector<float> texCoords;
            verts.reserve(12);
            texCoords.reserve(12);

            texCoords << 1.0 << 0.0; verts << tw  << 0.0; // Top right
            texCoords << 0.0 << 0.0; verts << 0.0 << 0.0; // Top left
            texCoords << 0.0 << 1.0; verts << 0.0 << sh;  // Bottom left
            texCoords << 0.0 << 1.0; verts << 0.0 << sh;  // Bottom left
            texCoords << 1.0 << 1.0; verts << tw  << sh;  // Bottom right
            texCoords << 1.0 << 0.0; verts << tw  << 0.0; // Top right
            GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
            vbo->reset();
            vbo->setData(6, 2, verts.constData(), texCoords.constData());
            vbo->render(GL_TRIANGLES);

            // At this point we don't need the scratch texture anymore
            tex.unbind();
            tex.discard();

            // create scratch texture for second rendering pass
            GLTexture tex2(tw, sh);
            tex2.setFilter(GL_LINEAR);
            tex2.setWrapMode(GL_CLAMP_TO_EDGE);
            tex2.bind();

            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - sh, tw, sh);

            // Set up the shader for vertical scaling
            float dy = sh / float(th);
            createKernel(dy, &kernelSize);
            createOffsets(kernelSize, m_offscreenTex->height(), Qt::Vertical);
            setUniforms();

            // Now draw the horizontally scaled window in the FBO at the right
            // coordinates on the screen, while scaling it vertically and blending it.
            glClear(GL_COLOR_BUFFER_BIT);

            verts.clear();

            verts << tw  << 0.0; // Top right
            verts << 0.0 << 0.0; // Top left
            verts << 0.0 << th;  // Bottom left
            verts << 0.0 << th;  // Bottom left
            verts << tw  << th;  // Bottom right
            verts << tw  << 0.0; // Top right
            vbo->setData(6, 2, verts.constData(), texCoords.constData());
            vbo->render(GL_TRIANGLES);

            tex2.unbind();
            tex2.discard();
            ShaderManager::instance()->popShader();

            // create cache texture
            GLTexture *cache = new GLTexture(tw, th);

            cache->setFilter(GL_LINEAR);
            cache->setWrapMode(GL_CLAMP_TO_EDGE);
            cache->bind();
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - th, tw, th);
            GLRenderTarget::popRenderTarget();

            if (hardwareClipping) {
                glEnable(GL_SCISSOR_TEST);
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            const qreal rgb = data.brightness() * data.opacity();
            const qreal a = data.opacity();

            ShaderBinder binder(ShaderManager::SimpleShader);
            GLShader *shader = binder.shader();
            shader->setUniform(GLShader::Offset, QVector2D(0, 0));
            shader->setUniform(GLShader::ModulationConstant, QVector4D(rgb, rgb, rgb, a));
            shader->setUniform(GLShader::Saturation, data.saturation());

            cache->render(region, textureRect, hardwareClipping);

            glDisable(GL_BLEND);

            if (hardwareClipping) {
                glDisable(GL_SCISSOR_TEST);
            }

            cache->unbind();
            w->setData(LanczosCacheRole, QVariant::fromValue(static_cast<void*>(cache)));

            // Delete the offscreen surface after 5 seconds
            m_timer.start(5000, this);
            return;
        }
    } // if ( effects->compositingType() == KWin::OpenGLCompositing )
    w->sceneWindow()->performPaint(mask, region, data);
} // End of function

void LanczosFilter::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        delete m_offscreenTarget;
        delete m_offscreenTex;
        m_offscreenTarget = 0;
        m_offscreenTex = 0;
        foreach (Client *c, Workspace::self()->clientList()) {
            discardCacheTexture(c->effectWindow());
        }
        foreach (Client *c, Workspace::self()->desktopList()) {
            discardCacheTexture(c->effectWindow());
        }
        foreach (Unmanaged *u, Workspace::self()->unmanagedList()) {
            discardCacheTexture(u->effectWindow());
        }
        foreach (Deleted *d, Workspace::self()->deletedList()) {
            discardCacheTexture(d->effectWindow());
        }
    }
}

void LanczosFilter::discardCacheTexture(EffectWindow *w)
{
    QVariant cachedTextureVariant = w->data(LanczosCacheRole);
    if (cachedTextureVariant.isValid()) {
        delete static_cast< GLTexture*>(cachedTextureVariant.value<void*>());
        w->setData(LanczosCacheRole, QVariant());
    }
}

void LanczosFilter::setUniforms()
{
    glUniform1i(m_uTexUnit, 0);
    glUniform2fv(m_uOffsets, 16, (const GLfloat*)m_offsets);
    glUniform4fv(m_uKernel, 16, (const GLfloat*)m_kernel);
}

} // namespace

