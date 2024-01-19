#ifndef NV12RENDER_H
#define NV12RENDER_H
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <QOpenGLDebugLogger>
#include <memory>
#include "decoder.h"
class Nv12Render : public QOpenGLFunctions
{
public:
    Nv12Render();
    ~Nv12Render();
    void init();
    void render();
    void resize(int width, int height);
    void updateTextureInfo(int width, int height, int format);
    void updateTextureData(const YUVData* data);

protected:
    void initTexture();
    void initShader();
    void initGeometry();

private:
    QOpenGLShaderProgram mProgram;
    QOpenGLTexture* mTexY = nullptr;
    QOpenGLTexture* mTexUV = nullptr;
    QVector<QVector3D> mVertices;
    QVector<QVector2D> mTexcoords;
    int mModelMatHandle, mViewMatHandle, mProjectMatHandle;
    int mVerticesHandle;
    int mTexCoordHandle;

    QMatrix4x4 mModelMatrix;
    QMatrix4x4 mViewMatrix;
    QMatrix4x4 mProjectionMatrix;
    GLint mPixFmt = 0;
    bool mTextureAlloced = false;

    QOpenGLDebugLogger* mLogger = nullptr;
};

#endif // NV12RENDER_H
