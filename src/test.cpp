#include <stdio.h>
#include <algorithm>
#include <pthread.h>
#include "libfreenect.hpp"
#include "lemming.h"
#include "level.h"
#include <iostream>
#include <vector>
using namespace std;

using namespace cv;
using namespace std;

class Mutex {
public:    
	Mutex() {
		pthread_mutex_init( &m_mutex, NULL );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
private:
	pthread_mutex_t m_mutex;
};

class MyFreenectDevice : public Freenect::FreenectDevice {
  public:
	MyFreenectDevice(freenect_context *_ctx, int _index)
		: Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(FREENECT_DEPTH_11BIT_SIZE),m_buffer_rgb(FREENECT_VIDEO_RGB_SIZE), m_gamma(2048), m_new_rgb_frame(false), m_new_depth_frame(false),
		  depthMat(Size(640,480),CV_16UC1), rgbMat(Size(640,480),CV_8UC3,Scalar(0)), ownMat(Size(640,480),CV_8UC3,Scalar(0))
	{
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			float v = i/2048.0;
			v = std::pow(v, 3)* 6;
			m_gamma[i] = v*6*256;
		}
	}
	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		std::cout << "RGB callback" << std::endl;
		m_rgb_mutex.lock();
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		rgbMat.data = rgb;
		m_new_rgb_frame = true;
		m_rgb_mutex.unlock();
	};
	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		std::cout << "Depth callback" << std::endl;
		m_depth_mutex.lock();
		uint16_t* depth = static_cast<uint16_t*>(_depth);
		depthMat.data = (uchar*) depth;
		m_new_depth_frame = true;
		m_depth_mutex.unlock();
	}

	bool getVideo(Mat& output) {
		m_rgb_mutex.lock();
		if(m_new_rgb_frame) {
			cv::cvtColor(rgbMat, output, CV_RGB2BGR);
			m_new_rgb_frame = false;
			m_rgb_mutex.unlock();
			return true;
		} else {
			m_rgb_mutex.unlock();
			return false;
		}
	}

	bool getDepth(Mat& output) {
			m_depth_mutex.lock();
			if(m_new_depth_frame) {
				depthMat.copyTo(output);
				m_new_depth_frame = false;
				m_depth_mutex.unlock();
				return true;
			} else {
				m_depth_mutex.unlock();
				return false;
			}
		}

  private:
	std::vector<uint8_t> m_buffer_depth;
	std::vector<uint8_t> m_buffer_rgb;
	std::vector<uint16_t> m_gamma;
	Mat depthMat;
	Mat rgbMat;
	Mat ownMat;
	Mutex m_rgb_mutex;
	Mutex m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
};
unsigned char tempUDepth[640*480];
unsigned char UDepth[640*480];
unsigned char UDepth2[640*480];
void proyectBody(unsigned char *depth, int tIni,int tEnd)
{
  for(int r=0;r<480;r++)
    for(int c=0;c<640;c++)
      if(  depth[r*640+c]>=tIni && depth[r*640+c]<=tEnd )
          UDepth[r*640+c]=0;
      else
       UDepth[r*640+c]=255;
}
void mouse(int event,int x,int y,int flag,void* cosa)
{
    if(event==CV_EVENT_MOUSEMOVE)
        printf("%d %d\n",x,y);
}

int main()
{
    level c("maskMap1.png");
    vector<lemming> g;
    int nog=10;
    for(int i=0;i<nog;i++)
    {
        g.push_back(lemming(10,50+i*30,5,14,15,1));
    }
    
    Mat depthMat(Size(640,480),CV_16UC1);
    Mat depthf  (Size(640,480),CV_8UC1);
    Mat rgbMat(Size(640,480),CV_8UC3,Scalar(0));
    Mat ownMat(Size(640,480),CV_8UC3,Scalar(0));

        Freenect::Freenect freenect;
        MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);
   
    cvSetMouseCallback("Draw",mouse);
    device.setTiltDegrees(0);
    device.startVideo();
    device.startDepth();
    
    int arr[]={-1,0,1,0,0};
    int arc[]={0,1,0,-1,0};
    
    while(1)
    {    
    
    	device.getVideo(rgbMat);
    	device.getDepth(depthMat);

        
    	depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
    	
    	
        for(int r=0;r<480;r++)
         for(int c=0;c<640;c++)
           tempUDepth[r*640+c]=depthf.at<unsigned char>(r,c);    	
    	
    	proyectBody(tempUDepth,50,115);

       // cv::imshow("Draw",depthf);
       
        c.copyData();
      
        c.merge(UDepth);
    
        for(int i=0;i<nog;i++)
                g[i].move(c); 
         
        c.draw(UDepth);
        
        for(int i=0;i<nog;i++)
                g[i].draw(c); 
        
        
        c.retData();
        
        cvShowImage("Draw",c.temp);  
        if(cvWaitKey(10)!=-1)
            break;
    }
    cvWaitKey(0);
    return 0;
}
