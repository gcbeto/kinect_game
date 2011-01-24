/*

 *

 * Redistribution and use in source and binary forms, with or without

 * modification, are permitted provided that the following conditions are met:

 *

 *     * Redistributions of source code must retain the above copyright

 *       notice, this list of conditions and the following disclaimer.

 *     * Redistributions in binary form must reproduce the above copyright

 *       notice, this list of conditions and the following disclaimer in the

 *       documentation and/or other materials provided with the distribution.

 *     * Neither the name of the <ORGANIZATION> nor the names of its

 *       contributors may be used to endorse or promote products derived from

 *       this software without specific prior written permission.

 *

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"

 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE

 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE

 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE

 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR

 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF

 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS

 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN

 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)

 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE

 * POSSIBILITY OF SUCH DAMAGE.

 */

/**
*	Author: GameCoder
*
*/

#ifndef LEVEL
#define LEVEL

#include <vector>
#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace std;

/* draw a pixel on a certain channel
* draw a sigle pixel on a cv::IPLImage
* @param IplImage Image to draw at
* @param x-y point coordinates
* @channel channel number to draw at
* @color intensity
*/
inline void pixel( IplImage*img, int y, int x, int channel, uchar val ) 
{
    (*(img->imageData + img->widthStep * y + x * img->nChannels + channel))=val;    
}

/* get the color of a pixel on a certain channel
* @param IplImage Image
* @param x-y point coordinates
* @channel channel number to ask for
* @return val of thepixel
*/

inline uchar pixel( IplImage*img, int y, int x, int channel ) 
{
    return (*(img->imageData + img->widthStep * y + x * img->nChannels + channel));    
}

/* class to hold the environment information 
*  @brief This is the map 
*  class to hold the information and draw functions of the environment
*  thresholds are readed from a white and black image.
*  environment is read froma a png image.
*/
class level
{
    public:
        const static int wall=0;
        const static int goal=128;
        
        int w,h;
        int **data;
        int **temporal;
        IplImage *temp;
        IplImage *color;
        IplImage *miscInit;
        vector<pair<int,int> >respawn;
        
        level(){}

	/* contructor
	* draw a sigle pixel on a cv::IPLImage
	* @param mask array of colition zones
	* @param file png file for enviromment colors and scenario.
	*/
        level(string mask,string file)
        {
            IplImage *img=cvLoadImage(mask.c_str());
            color=cvLoadImage(file.c_str());
            
            miscInit=cvLoadImage("initMisc.png");
            
           // ROS_INFO("%d %d",miscInit->height,miscInit->width);
            
            h=img->height;
            w=img->width;
            
            data=new int*[h];
            temporal=new int*[h];
            
            for(int i=0;i<h;i++)
            {
                data[i]=new int[w];
                temporal[i]=new int[w];
                for(int k=0;k<w;k++)
                {
                    if(img->nChannels>1)
                    {
                        data[i][k]=(pixel(img,i,k,0)+pixel(img,i,k,1)+pixel(img,i,k,2))/3;
                        if(pixel(img,i,k,0)==0 && pixel(img,i,k,1)==255 && pixel(img,i,k,0)==0)
                        {
                        	respawn.push_back(make_pair(k,i));
                        }
                    }
                    else
                        data[i][k]=pixel(img,i,k,0);
                }
            }
            
            temp=cvCreateImage(cvSize(w,h), 8, 3);
            //temp=img;
            cvNamedWindow("Draw",0);
            //cvShowImage("Draw",img);
            
            copyData();
        }


	/* 
	*draw miscelaniuos objects.
	*/        
        void drawMiscs()
        {
        	for(int i=0;i<respawn.size();i++)
        	{
        		drawImg(miscInit,respawn[i].first-20,respawn[i].second-20);
		}
        }

        /* 
	*draw a custom image.
	*@param img image to draw.
	*@param x-y position
	*/  
        void drawImg(IplImage *img,int x,int y)
        {
        	for(int i=0;i<img->height;i++)
        	{
        		for(int k=0;k<img->width;k++)
        		{
        			if(!(pixel(img,i,k,2)<100 && pixel(img,i,k,1)<100 && pixel(img,i,k,0)>128))
        			for(int r=0;r<img->nChannels;r++)
        			{
        				if(y+i>=0 && x+k>=0 && y+i<temp->height && x+k<temp->width)
        				{
	        				pixel(temp,y+i,x+k,r,pixel(img,i,k,r));
	        			}
        			}
        		}
        	}
	}
        
        
	/* 
	*draw the collision BW mask and the scenario
	*@param body 
	*@param x-y position
	*/  

        void draw(unsigned char *mask)
        {
            for(int i=0;i<h;i++)
            {
                for(int k=0;k<w;k++)
                {
                    int tI=(int)(((float)i/(float)h)*480.0);
                    int tK=640-(int)(((float)k/(float)w)*640.0);
                    int  i2=i,k2=k;
                     float consR=0.0,consG=0.0,consB=0.0;
			if(mask[tI*640+tK]==0)consR=128,consG=128,consB=128,i2=max(0,i2-10),k2=max(0,k2-10);

			pixel(temp,i,k,0,min(pixel(color,i2,k2,0)+consB,255.0f));
			pixel(temp,i,k,1,min(pixel(color,i2,k2,1)+consG,255.0f));
			pixel(temp,i,k,2,min(pixel(color,i2,k2,2)+consR,255.0f));
                }
            }
        }
        

	/* 
	*copy the mask information into a colition array
	*@param body collision array
	*@param 
	*/  
        void merge(unsigned char *body)
        {
             for(int i=0;i<h;i++)            
              for(int k=0;k<w;k++)
               {
                    int tI=(int)(((float)i/(float)h)*480.0);
                    int tK=640-(int)(((float)k/(float)w)*640.0);
                    data[i][k]*=(body[tI*640+tK]/255);
               }
        }

        void retData()
        {
          for(int i=0;i<h;i++)            
              for(int k=0;k<w;k++)
               {
                    data[i][k]=temporal[i][k];
               }
        }

       void copyData()
        {
          for(int i=0;i<h;i++)            
              for(int k=0;k<w;k++)
               {
                    temporal[i][k]=data[i][k];
               }
        }
};

#endif
