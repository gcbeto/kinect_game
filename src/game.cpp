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
*	Author: Alberto Jose Ramirez Valadez     @gcbeto
**/

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/Image.h"
#include "cv_bridge/CvBridge.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "lemming.cpp"
#include <stdlib.h>

sensor_msgs::CvBridge bridge_;			// Bridge to transfor sensor_msgs to IplImage  (opencv)
level c;					// Class of a Level of the game
vector<lemming> g;				// vector of the autonomous robots kinemmings
int nog=0;					// deprecated
int threshold=0;				// Threshold to the depth buffer of the kinect
double t=0;					// variable to store the time lapsed
double tt=0;					// variable to know how to much time have passed
CvFont font;					// font for the text

/* variables for the level */
int alive=0;						
int dead=0;
int rescued=0;
int maxlem=0;
int needed=0;
double tim=0;

string masks[10];
string files[10];
int neededs[10];
int maxlems[10];
double ltimes[10];
int nolevel=0;
int maxlevel=10;
/* end of variables for the level */


unsigned char UDepth[640*480];			// Depth Buffer


void proyectBody(char *depth, int tIni,int tEnd)	//Funtion that projects the "body" to the scenario
{
  for(int r=0;r<480;r++)
    for(int c=0;c<640;c++)
    {
      if(  depth[r*640+c]>=tIni && depth[r*640+c]<=tEnd )
       UDepth[r*640+c]=0;
      else
       UDepth[r*640+c]=255;
    }
}

void initGame(string mask,string file,int MAXLEM=30,int NEED=10,int LTIME=60)		//Initialize the game. Param= Specification for the level
{
	c=level(mask,file);
	g.clear();
	alive=0;
	dead=0;
	rescued=0;
	needed=NEED;
	tim=LTIME;
	maxlem=MAXLEM;
	t=0;
	tt=0;
}

void gameLoop()				//Loop of the game (drawing, moving, timing, etc...)
{
   ros::WallTime ttt=ros::WallTime::now();	//calculate time
   
   //for(int i=0;i<640*480;i++)
  	//UDepth[i]=255;
  	
      
   c.merge(UDepth);				// merge the projection body with the map
   
   if(t>tim)					// check timeout
   {
   	ROS_INFO("\033[32mYou lose :(\n\031[0m");
   	initGame(masks[nolevel],files[nolevel],neededs[nolevel]);
   }
   
   if(t-tt>1 && alive<maxlem)			// spawn new kinemming  (1 sec/kinemming)
   {
   	tt=t;
   	for(int i=0;i<c.respawn.size();i++)
   	{
   		g.push_back(lemming(10,c.respawn[i].first,c.respawn[i].second,15,25,1));
   		alive=g.size();
   	}
   }
   
   for(int i=0;i<g.size();i++)			// check if the kinemmings are inside the game
   	if(!g[i].inGame(c))
   	{
   		g.erase(g.begin()+i);
   		i--;
   		dead++;
   		alive=g.size();
	}

    for(int i=0;i<g.size();i++)			// check if the kinemmings are inside the goal
   	if(g[i].inGoal(c))
   	{
   		g.erase(g.begin()+i);
   		i--;
   		rescued++;
   		alive=g.size();
   		if(rescued>=needed)
   		{
   			nolevel++;
   			if(nolevel>=maxlevel)
   			{
   				ROS_INFO("\033[33mWuuu You Won All the Levels!!!\n\031[0m");
   			}else
   			{
   				initGame(masks[nolevel],files[nolevel],neededs[nolevel]);
   				ROS_INFO("\033[34mWuuu You Won!!!\n\033[0m");
   			}
		}
	}
   
   for(int i=0;i<g.size();i++)			// Move all the kinemmings
     g[i].move(c); 
     
     c.draw(UDepth);				// Draw your body
        
   for(int i=0;i<g.size();i++)			// Draw all the kinemmings
     g[i].draw(c); 
     
   c.retData();					// Resotre data of the level (cleaning depthbuffer)
   
   c.drawMiscs();				// Draw misc things
   
 
  /*Draw text*/
  char text[50];
  sprintf(text,"Alive: %d/%d",alive,maxlem);
  cvPutText (c.temp,text,cvPoint(0,20), &font, cvScalar(0,255,0));
  sprintf(text,"Dead: %d",dead);
  cvPutText (c.temp,text,cvPoint(180,20), &font, cvScalar(0,0,255));
  sprintf(text,"Rescued: %d/%d",rescued,needed);
  cvPutText (c.temp,text,cvPoint(300,20), &font, cvScalar(255,128,128));
  sprintf(text,"Time: %.1lf/%.1lf",t,tim);
  cvPutText (c.temp,text,cvPoint(500,20), &font, cvScalar(0,255,255));
  
  cvShowImage("Draw", c.temp);			// Show final image
  
  t+=(ros::WallTime::now()-ttt).toSec();	// Calculate time
}

void chatterCallback(const std_msgs::String::ConstPtr& msg)	// Deprecated Function
{
  
  gameLoop();
  
  cvWaitKey(2);
  
}

void cosa(const sensor_msgs::Image::ConstPtr& msg)		// Subscribe function, it is subcribed to /kinect/depth/image_raw
{
  ros::WallTime ttt=ros::WallTime::now();
	
  IplImage *img=bridge_.imgMsgToCv(msg, "mono8");		// Obtain depth buffer from kinect
  
  proyectBody(img->imageData,0,15+threshold);
  
  gameLoop();
  
  /*up arrow and down arrow: change the threshold for the depth buffer*/
  int key=cvWaitKey(2);
  if(key==1113938)
  {
  	threshold++;
  	ROS_INFO("%d",threshold);
  }
  if(key==1113940)
  {
  	threshold--;
  	ROS_INFO("%d",threshold);
  }
  
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "game");			// Initialize ROS

  ros::NodeHandle n;					// Node handler for ROS
  
  cvInitFont(&font,CV_FONT_HERSHEY_TRIPLEX, .75,.75,0,1.5);	// Create a Font (for the text in screen)

  /* Read information about the levels in the game */
  FILE*in=fopen("levels","r");
  fscanf(in,"%d",&maxlevel);
  for(int i=0;i<maxlevel;i++)
  {
  	char text[100];
  	fscanf(in,"%s",text);
  	masks[i]=text;
  	fscanf(in,"%s",text);
  	files[i]=text;
  	fscanf(in,"%d%d%lf",&maxlems[i],&neededs[i],&ltimes[i]);
  	ROS_INFO("Reading level %d",i);
  }
  fclose(in);
  
  
  initGame(masks[nolevel],files[nolevel],neededs[nolevel]);		// Initialize first level
  
  /* Change window position and size */
    cvMoveWindow("Draw",0,0);
    cvResizeWindow("Draw",1024,768);
    
    
  ros::Subscriber sub = n.subscribe("kinect/depth/image_raw", 5, cosa);		// Subscribe to the topic of the kinect driver
  //ros::Subscriber sub = n.subscribe("chatter", 10, chatterCallback);

  ros::spin();								// Spin of ROS

  return 0;
}

