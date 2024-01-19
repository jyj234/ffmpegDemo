#include "i420render.h"

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

I420Render::I420Render()
{
}

I420Render::~I420Render()
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
   safeDeleteTexture(mTexU);
   safeDeleteTexture(mTexV);
}

void I420Render::init()
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
void I420Render::resize(int width, int height)
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
void I420Render::initShader()
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
void I420Render::initTexture()
{
   // yuv420p
   mTexY = new QOpenGLTexture(QOpenGLTexture::Target2D);
   mTexY->setFormat(QOpenGLTexture::LuminanceFormat);
   //    mTexY->setFixedSamplePositions(false);
   mTexY->setMinificationFilter(QOpenGLTexture::Nearest);
   mTexY->setMagnificationFilter(QOpenGLTexture::Nearest);
   mTexY->setWrapMode(QOpenGLTexture::ClampToEdge);

   mTexU = new QOpenGLTexture(QOpenGLTexture::Target2D);
   mTexU->setFormat(QOpenGLTexture::LuminanceFormat);
   //    mTexU->setFixedSamplePositions(false);
   mTexU->setMinificationFilter(QOpenGLTexture::Nearest);
   mTexU->setMagnificationFilter(QOpenGLTexture::Nearest);
   mTexU->setWrapMode(QOpenGLTexture::ClampToEdge);

   mTexV = new QOpenGLTexture(QOpenGLTexture::Target2D);
   mTexV->setFormat(QOpenGLTexture::LuminanceFormat);
   //    mTexV->setFixedSamplePositions(false);
   mTexV->setMinificationFilter(QOpenGLTexture::Nearest);
   mTexV->setMagnificationFilter(QOpenGLTexture::Nearest);
   mTexV->setWrapMode(QOpenGLTexture::ClampToEdge);
}

void I420Render::initGeometry()
{
   mVertices << QVector3D(-1, 1, 0.0f) << QVector3D(1, 1, 0.0f) << QVector3D(1, -1, 0.0f) << QVector3D(-1, -1, 0.0f);
   mTexcoords << QVector2D(0, 1) << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

   mViewMatrix.setToIdentity();
   mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.001f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));
   mModelMatrix.setToIdentity();
}
void I420Render::updateTextureInfo(int width, int height, int format)
{
   if (format == AV_PIX_FMT_YUV420P)
   {
        qDebug()<<"format == AV_PIX_FMT_YUV420P";
       // yuv420p
       mTexY->setSize(width, height);
       mTexY->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

       mTexU->setSize(width / 2, height / 2);
       mTexU->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

       mTexV->setSize(width / 2, height / 2);
       mTexV->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);
   }
   else
   {
       // 先按yuv444p处理
       mTexY->setSize(width, height);
       mTexY->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

       mTexU->setSize(width, height);
       mTexU->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

       mTexV->setSize(width, height);
       mTexV->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);
   }
   mTextureAlloced = true;
}

void I420Render::updateTextureData(const YUVData* data)
{
   QOpenGLPixelTransferOptions options;
    options.setImageHeight(data->h);
   options.setRowLength(data->yLineSize);
    mTexY->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, data->Y, &options);
   options.setImageHeight(data->h / 2);
   options.setRowLength(data->uLineSize / 2);
    mTexU->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, data->U, &options);
   options.setRowLength(data->vLineSize / 2);
    mTexV->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, data->V, &options);
}
void I420Render::render()
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

   // U
   glActiveTexture(GL_TEXTURE1);
   mTexU->bind();

   // V
   glActiveTexture(GL_TEXTURE2);
   mTexV->bind();

   mProgram.setUniformValue("tex_y", 0);
   mProgram.setUniformValue("tex_u", 1);
   mProgram.setUniformValue("tex_v", 2);

   glDrawArrays(GL_TRIANGLE_FAN, 0, mVertices.size());

   mProgram.disableAttributeArray(mVerticesHandle);
   mProgram.disableAttributeArray(mTexCoordHandle);
   mProgram.release();
}
