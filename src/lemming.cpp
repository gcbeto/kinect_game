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

#ifndef LEMMING
#define LEMMING

#include "level.cpp"


const double gravity=6; //< Glocal gravity of the lemmings
const double velocity=4;//< Global velocity of the lemmings

/**
* Lemmings class
*/

class lemming
{
    public:
        const static int down=1;     //< Falling constant
        const static int left=1<<1;  //< Left constant 
        const static int right=1<<2; //< right constant
        
        
        double hp;                   //<Lemming HP
        double x,y;                  //<Lemming position
        double w,h;                  //<Lemming size
        double dir;                  //<Lemming direction
        float time;                  //<Lemming time
        IplImage *sprite;            //<Lemming sprite actual image has 10 frames
        
        /**
        * Ask if the lemming is inside. ie is alive
        */
        bool inGame(level &game)     
        {
            if((!insideGame(y,x-1,game) || !insideGame(y,x+w,game) || 1) &&  !insideGame(y+h+1,x+w/2,game))
                return false;
            return true;
        }
        
        /**
        *Ask if the lemming is in the goal
        */
        
        bool inGoal(level &game)
        {
        	if(inGame(game))
        	{
        		if(game.data[(int)(y+h/2)][(int)(x+w/2)]==level::goal)
        			return true;
		}
		return false;
        }
        /**
        * Ask if the lemming is inside. ie is alive
        */        
        bool insideGame(double y,double x,level &game)
        {
            if(y>=0 && y<game.h && x>=0 && x<game.w)
            {
                return true;
            }
            return false;
        }
        /**
        * Collision detection
        * Per pixel collision
        */        
        int checkCollision(level &game)
        {
            int res=0;
            if(y+h+1<game.h)                                             //Check colision front        
            {
                for(int i=0;i<w;i++)
                {
                    if(game.data[(int)(y+h+1)][(int)(x+i)]==level::wall)
                    {
                        res|=down;                                        //Falling case
                        break;
                    }
                }
            }
            if(y>=0)
            for(int i=0;i<h/3*2;i++)
            {
                if(x-1>=0 && game.data[(int)(y+i)][(int)(x-1)]==level::wall)
                {
                    res|=left;                                                        //Turn to left case
                }
                if(x+w+1<game.w && game.data[(int)(y+i)][(int)(x+w+1)]==level::wall)
                {
                    res|=right;                                                       //Turn to right case
                }
            }
            
            return res;
        }
        
    public:
    /**
    * Simple constructor
    */
        lemming(){}
    /**
    * Full constructor
    */        
        lemming(double HP,double X,double Y,double W,double H,double DIR)
        {
            hp=HP;
            x=X;
            y=Y;
            w=W;
            h=H;
            dir=DIR*velocity;
            time=0.0;
            sprite=cvLoadImage("sprite.bmp");
        }
        
        /**
        * Function that moce the lemming
        */
        void move(level &game)
        {
            if(!inGame(game)) return;                     //Is alive?
            
            int col=checkCollision(game);                 //Collision check
            
            if(!col&down)                                 //Falling case
            {
                y+=gravity;
            }else
            {
                bool flag=0;
                if(col&left)                             //Left case
                {
                    dir=velocity;
                    x+=dir;
                    flag=1;
                }
                if(col&right)                            //Right case
                {
                    dir=-velocity;
                    x+=dir;
                    flag=1;
                }
                if(!flag)                                //Move if is possible
                {
                    x+=dir;
                }
                time+=.8;
                for(int i=0;i<2*h/3;i++)                //
                {
                    if(game.data[(int)(y+i)][(int)(x+w/2)]==level::wall)
                    {
                    	return;
                    }
                }
                for(int i=2*h/3;i<h;i++)
                {
                    if(game.data[(int)(y+i)][(int)(x+w/2)]==level::wall)
                    {
                        y-=h-i;
                        break;
                    }
                }
            }
            
            //printf("%lf %lf %d %lf\n",x,y,col,dir);
        }
        /**
        *Draw the lemming with the sprite, 255,255,255 is transparent
        */
        void draw(level &game)
        {   
            for(int i=(int)(y);i<y+h;i++)
            {
                for(int k=(int)(x);k<x+w;k++)
                {
                    if(insideGame(i,k,game))
                    {
                       int R=(int)(((float)(i-y)/(float)h)*(float)sprite->height);
                       int C=(int)(((float)(k-x)/(float)w)*(float)(sprite->width/10.0))+(float)(sprite->width/10.0)*((int)(time)%10);
                                              
                       int rc=pixel(sprite,R,C,0),gc=pixel(sprite,R,C,1),bc=pixel(sprite,R,C,2);
                       if(rc<220 || gc<220 ||bc<220)          //Transparent
                       {
                         if(dir>0)
                         {
                          pixel(game.temp,i,k,0,rc);            //Right direction
                          pixel(game.temp,i,k,1,gc);
                          pixel(game.temp,i,k,2,bc);
                         }else                                   //Left direction
                         {
                          pixel(game.temp,i,(w-(k-x))+x,0,rc);
                          pixel(game.temp,i,(w-(k-x))+x,1,gc);
                          pixel(game.temp,i,(w-(k-x))+x,2,bc);                         
                         }
                        }

                    }
                }
            }
        }
};

#endif
