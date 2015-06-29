#include "glwidget.hpp"
#include <QDebug>

GLWidget::GLWidget(QWidget *parent) : Tucano::QtPlainWidget(parent)
{
	initd = false;

    ROIcorner = Eigen::Vector2i(0,0);
    ROIspread = Eigen::Vector2i(10,10);
}

GLWidget::~GLWidget()
{

}

void GLWidget::initialize (void)
{
	/// resize OpenGL viewport to current size
    glViewport(0, 0, this->width(), this->height());

    ///initialize frame container
    //frame = new cv::Mat;

    // the default is /shaders from your running dir
    string shaders_dir("./shaders/");

    rendertexture.setShadersDir(shaders_dir);
    rendertexture.initialize();
    rect.setShadersDir(shaders_dir);
    rect.initialize();
    meanShift.setShadersDir(shaders_dir);
    meanShift.initialize();

    this->resizeGL(10, 10);

	//frameTextureID = frameTexture.create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
    frameTexture = new Tucano::Texture();
    //temporary


    /// set this widget as initialized
    initd = true;
}

void GLWidget::paintGL (void)
{
	if(!initd)
        return;

	makeCurrent();

    static const struct {
      unsigned int   width;
      unsigned int   height;
      unsigned int   bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
      unsigned char  pixel_data[10 * 10 * 4 + 1];
    } frame_one = {
      10, 10, 4,
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
      "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\377\0\0\377\0\0\0\377\0\0"
      "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0"
      "\377\0\0\377\377\377\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
      "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\377\377\0\0\377\377\377\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\377"
      "\0\0\377\0\0\377\377\0\0\377\377\377\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377",
    };

    static const struct {
      unsigned int   width;
      unsigned int   height;
      unsigned int   bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
      unsigned char  pixel_data[10 * 10 * 4 + 1];
    } frame_two = {
      10, 10, 4,
      "\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\377\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\377\377\377"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
      "\0\377\0\0\0\377\0\0\377\377\0\0\377\377\377\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\377\0\0\377\0\0\377\377"
      "\0\0\377\377\377\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\377\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377"
      "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0"
      "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
      "\0\377\0\0\0\377",
    };

    static const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char  pixel_data[4 * 4 * 4 + 1];
} frame3 = {
  4, 4, 4,
  "\20\20\20\377\40\40\40\377000\377@@@\377\220\220\220\377ppp\377```\377PP"
  "P\377\240\240\240\377\260\260\260\377\300\300\300\377\320\320\320\377\0\0"
  "\0\377\377\377\377\377\360\360\360\377\340\340\340\377",
};

    /*
    for(int i = 0; i<100; i++){
      std::cout<<" ("<<(int)frame_one.pixel_data[4*i]
        <<", "<<(int)frame_one.pixel_data[(4*i)+1]
        <<", "<<(int)frame_one.pixel_data[(4*i)+2]
        <<", "<<(int)frame_one.pixel_data[(4*i)+3]<<") ,";
    }
    std::cout<<"\n";
    for(int i = 0; i<100; i++){
      std::cout<<" ("<<(int)frame_two.pixel_data[4*i]
        <<", "<<(int)frame_two.pixel_data[(4*i)+1]
        <<", "<<(int)frame_two.pixel_data[(4*i)+2]
        <<", "<<(int)frame_two.pixel_data[(4*i)+3]<<") ,";
    }
    std::cout<<std::endl;
    */
	
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	frameTexture->create(GL_TEXTURE_2D, GL_RGBA8, frame_one.width, frame_one.height, GL_RGBA, GL_UNSIGNED_BYTE, frame_one.pixel_data);
	

	// renders the given image, not that we are setting a fixed viewport that follows the widgets size
    // so it may not be scaled correctly with the image's size (just to keep the example simple)
    Eigen::Vector2i viewport (10,10);//(this->width(), this->height());
    rendertexture.renderTexture(*frameTexture, viewport);//debug commented
        
    meanShift.setRegionDimensionsAndCenter(viewport, ROIcorner, ROIspread);
    meanShift.histogramQ(frameTexture);
    
    frameTexture->create(GL_TEXTURE_2D, GL_RGBA8, frame_two.width, frame_two.height, GL_RGBA, GL_UNSIGNED_BYTE, frame_two.pixel_data);
    
    meanShift.histogramP(frameTexture);
    meanShift.meanshift(&ROIcorner, &ROIspread);

    rect.renderTexture(viewport, ROIcorner, ROIspread);
    
    initd = false;

    update();
}