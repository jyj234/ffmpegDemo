#ifndef I420RENDER_H
#define I420RENDER_H


#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <QOpenGLDebugLogger>
#include <memory>
#include "decoder.h"
class I420Render : public QOpenGLFunctions
{
public:
    I420Render();
    ~I420Render();
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
    QOpenGLTexture* mTexU = nullptr;
    QOpenGLTexture* mTexV = nullptr;
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


#endif // I420RENDER_H
