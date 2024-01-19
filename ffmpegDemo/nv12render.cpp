#include "nv12render.h"

#include <QOpenGLPixelTransferOptions>
static void safeDeleteTexture(QOpenGLTexture* texture)
{
    if (texture)
    {
        if (texture->isBound())
        {
            texture->release();
        }
        if (texture->isCreated())
        {
            texture->destroy();
        }
        delete texture;
        texture = nullptr;
    }
}

Nv12Render::Nv12Render()
{
}

Nv12Render::~Nv12Render()
{
#ifdef DEBUG_GL
    if(mLogger)
    {
        mLogger->stopLogging();
        delete mLogger;
        mLogger = nullptr;
    }
#endif
    safeDeleteTexture(mTexY);
    safeDeleteTexture(mTexUV);
}

void Nv12Render::init()
{
    qWarning() << __FUNCTION__;
    initializeOpenGLFunctions();
#ifdef DEBUG_GL
    mLogger = new QOpenGLDebugLogger();
    QObject::connect(mLogger, &QOpenGLDebugLogger::messageLogged, mLogger, [](const QOpenGLDebugMessage &debugMessage){
        qWarning() << "GL:" << debugMessage.type() << debugMessage.message();
    });
    mLogger->initialize();
    mLogger->startLogging();
#endif
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    initShader();
    initTexture();
    initGeometry();
}
void Nv12Render::resize(int width, int height)
{
    qWarning() << __FUNCTION__;
    glViewport(0, 0, width, height);
    float bottom = -1.0f;
    float top = 1.0f;
    float n = 1.0f;
    float f = 100.0f;
    mProjectionMatrix.setToIdentity();
    mProjectionMatrix.frustum(-1.0, 1.0, bottom, top, n, f);
}
void Nv12Render::initShader()
{
    if (!mProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/vertex.vsh"))
    {
        qWarning() << " add vertex shader file failed.";
        return;
    }
    if (!mProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/fragment.fsh"))
    {
        qWarning() << " add fragment shader file failed.";
        return;
    }
    mProgram.bindAttributeLocation("qt_Vertex", 0);
    mProgram.bindAttributeLocation("texCoord", 1);
    mProgram.link();
    mProgram.bind();
}
void Nv12Render::initTexture()
{
    // yuv420p
    mTexY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTexY->setFormat(QOpenGLTexture::R8_UNorm);
    //    mTexY->setFixedSamplePositions(false);
    mTexY->setMinificationFilter(QOpenGLTexture::Nearest);
    mTexY->setMagnificationFilter(QOpenGLTexture::Nearest);
    mTexY->setWrapMode(QOpenGLTexture::ClampToEdge);

    mTexUV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTexUV->setFormat(QOpenGLTexture::RG8_UNorm);
    //    mTexU->setFixedSamplePositions(false);
    mTexUV->setMinificationFilter(QOpenGLTexture::Nearest);
    mTexUV->setMagnificationFilter(QOpenGLTexture::Nearest);
    mTexUV->setWrapMode(QOpenGLTexture::ClampToEdge);
}

void Nv12Render::initGeometry()
{
    mVertices << QVector3D(-1, 1, 0.0f) << QVector3D(1, 1, 0.0f) << QVector3D(1, -1, 0.0f) << QVector3D(-1, -1, 0.0f);
    mTexcoords << QVector2D(0, 1) << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    mViewMatrix.setToIdentity();
    mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.001f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mModelMatrix.setToIdentity();
}
void Nv12Render::updateTextureInfo(int width, int height, int format)
{
    if (format == AV_PIX_FMT_NV12)
    {
        qDebug()<<"format == AV_PIX_FMT_NV12";
        mPixFmt = 1;
        // NV12
        mTexY->setSize(width, height);
        mTexY->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

        mTexUV->setSize(width / 2, height / 2);
        mTexUV->allocateStorage(QOpenGLTexture::RG, QOpenGLTexture::UInt8);
    }
    mTextureAlloced = true;
}

void Nv12Render::updateTextureData(const YUVData* data)
{
    QOpenGLPixelTransferOptions options;
    options.setImageHeight(data->h);
    options.setRowLength(data->yLineSize);
    mTexY->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, data->Y, &options);
    options.setImageHeight(data->h / 2);
    options.setRowLength(data->uLineSize / 2);
    mTexUV->setData(QOpenGLTexture::RG, QOpenGLTexture::UInt8, data->U, &options);
}
void Nv12Render::render()
{
    glDepthMask(true);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!mTextureAlloced)
    {
        return;
    }
    mProgram.bind();

    mModelMatHandle = mProgram.uniformLocation("u_modelMatrix");
    mViewMatHandle = mProgram.uniformLocation("u_viewMatrix");
    mProjectMatHandle = mProgram.uniformLocation("u_projectMatrix");
    mVerticesHandle = mProgram.attributeLocation("qt_Vertex");
    mTexCoordHandle = mProgram.attributeLocation("texCoord");
    //顶点
    mProgram.enableAttributeArray(mVerticesHandle);
    mProgram.setAttributeArray(mVerticesHandle, mVertices.constData());

    //纹理坐标
    mProgram.enableAttributeArray(mTexCoordHandle);
    mProgram.setAttributeArray(mTexCoordHandle, mTexcoords.constData());

    // MVP矩阵
    mProgram.setUniformValue(mModelMatHandle, mModelMatrix);
    mProgram.setUniformValue(mViewMatHandle, mViewMatrix);
    mProgram.setUniformValue(mProjectMatHandle, mProjectionMatrix);

    // pixFmt
    mProgram.setUniformValue("pixFmt", mPixFmt);

    //纹理
    // Y
    glActiveTexture(GL_TEXTURE0);
    mTexY->bind();

    // UV
    glActiveTexture(GL_TEXTURE1);
    mTexUV->bind();


    mProgram.setUniformValue("tex_y", 0);
    mProgram.setUniformValue("tex_uv", 1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, mVertices.size());

    mProgram.disableAttributeArray(mVerticesHandle);
    mProgram.disableAttributeArray(mTexCoordHandle);
    mProgram.release();
}
