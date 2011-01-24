#ifndef LEMMING
#define LEMMING

#include "level.cpp"


const double gravity=6;
const double velocity=4;

class lemming
{
    public:
        const static int down=1;
        const static int left=1<<1;
        const static int right=1<<2;
        
        
        double hp;
        double x,y;
        double w,h;
        double dir;
        float time;
        IplImage *sprite;
        
        bool inGame(level &game)
        {
            if((!insideGame(y,x-1,game) || !insideGame(y,x+w,game) || 1) &&  !insideGame(y+h+1,x+w/2,game))
                return false;
            return true;
        }
        
        bool inGoal(level &game)
        {
        	if(inGame(game))
        	{
        		if(game.data[(int)(y+h/2)][(int)(x+w/2)]==level::goal)
        			return true;
		}
		return false;
        }
        
        bool insideGame(double y,double x,level &game)
        {
            if(y>=0 && y<game.h && x>=0 && x<game.w)
            {
                return true;
            }
            return false;
        }
        
        int checkCollision(level &game)
        {
            int res=0;
            if(y+h+1<game.h)
            {
                for(int i=0;i<w;i++)
                {
                    if(game.data[(int)(y+h+1)][(int)(x+i)]==level::wall)
                    {
                        res|=down;
                        break;
                    }
                }
            }
            if(y>=0)
            for(int i=0;i<h/3*2;i++)
            {
                if(x-1>=0 && game.data[(int)(y+i)][(int)(x-1)]==level::wall)
                {
                    res|=left;
                }
                if(x+w+1<game.w && game.data[(int)(y+i)][(int)(x+w+1)]==level::wall)
                {
                    res|=right;
                }
            }
            
            return res;
        }
        
    public:
        lemming(){}
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
        
        void move(level &game)
        {
            if(!inGame(game)) return;
            
            int col=checkCollision(game);
            
            if(!col&down)
            {
                y+=gravity;
            }else
            {
                bool flag=0;
                if(col&left)
                {
                    dir=velocity;
                    x+=dir;
                    flag=1;
                }
                if(col&right)
                {
                    dir=-velocity;
                    x+=dir;
                    flag=1;
                }
                if(!flag)
                {
                    x+=dir;
                }
                time+=.8;
                for(int i=0;i<2*h/3;i++)
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
                       
//                       if(dir<0)C=(float)(sprite->width/10.0)-(int)(((float)(k-x)/(float)w)*(float)(sprite->width/10.0))+(float)(sprite->width/10.0)*((int)(time)%10);
                       
                       int rc=pixel(sprite,R,C,0),gc=pixel(sprite,R,C,1),bc=pixel(sprite,R,C,2);
                       if(rc<220 || gc<220 ||bc<220)
                       {
                         if(dir>0)
                         {
                          pixel(game.temp,i,k,0,rc);
                          pixel(game.temp,i,k,1,gc);
                          pixel(game.temp,i,k,2,bc);
                         }else
                         {
                          pixel(game.temp,i,(w-(k-x))+x,0,rc);
                          pixel(game.temp,i,(w-(k-x))+x,1,gc);
                          pixel(game.temp,i,(w-(k-x))+x,2,bc);                         
                         }
                        }
                       // pixel(game.temp,i,k,0,0);
                       // pixel(game.temp,i,k,1,255);
                       // pixel(game.temp,i,k,2,0);
                    }
                }
            }
        }
};

#endif
