#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include<unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define RET  "\x1B[0m"

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"

#define WHT  "\x1B[37m"

struct ball{
	float x;
	float y;
	float radius;
	float dx;
	float dy;
	float h;
	float s;
	float v;
};
struct colour{
	float h;
	float s;
	float v;
};

//simple ball maths
float val(int x, int y, struct ball b){
	return pow(b.radius,2)/(pow((int)(b.x-x),2)+pow((int)(b.y-y),2));
}

//generate a ball
struct ball generateBall(int w, int h, float simspd,float radMod){
	int x = rand() % w;
	int y = rand() % h;
	float rad=( (float)rand()/(float)(RAND_MAX/5)+2)*radMod;
	float dx= ((float)rand()/(float)(RAND_MAX/.5)-.25)*simspd;
	float dy= ((float)rand()/(float)(RAND_MAX/.5)-.25)*simspd;

	float hue= (float)rand()/(float)(RAND_MAX/2);
	float sat= (float)rand()/(float)(RAND_MAX/1);
	float val= (float)rand()/(float)(RAND_MAX/1)*1;
	struct ball ball={x,y, rad,dx,dy, hue,sat,val};
	
	return ball;
}
//update ball positions
void updateBalls(struct ball *o, int balls, int w, int h){
		for(int i = 0; i< balls;i++){
			
			o[i].y+=o[i].dy;
			o[i].x+=o[i].dx;
			if(o[i].x>w){
				o[i].dx*=-1;
				o[i].x=w;
			}
			else if(o[i].x<0){
				o[i].dx*=-1;
				o[i].x=0;
			}
			if(o[i].y>h){
				o[i].dy*=-1;
				o[i].y=h;
			}
			else if(o[i].y<0){
				o[i].dy*=-1;
				o[i].y=0;
			}
		}
}
//hue to ascii colour thingies
const char* colToTcol(struct colour v,float tresh, float mtresh){
	if(v.s<.3){
		return WHT;
	}
	if(fmod(v.h,1)>5.0/6){
		return RED;	
	}else if(fmod(v.h,1)>4.0/6){
		return MAG;	
	}else if(fmod(v.h,1)>3.0/6){
		return BLU;	
	}else if(fmod(v.h,1)>2.0/6){
		return CYN;	
	}else if(fmod(v.h,1)>1.0/6){
		return GRN;	
	}else if(fmod(v.h,1)>=0.0){
		return YEL;	
	}
}
//render the cells  to terminal
void drawScreen(struct colour* s, int w, int h){
	for (int yy = 0; yy < h; yy++){
		for (int xx = 0; xx < w; xx++){
			struct colour celCol = s[xx+yy*w];
			
			const char* c = colToTcol(celCol,1.0, 1.5);
			char toPrint=' ';	
			if(celCol.v>1.5){
				toPrint=' ';
			}else if(celCol.v>1.1){
				toPrint='.';
			}else if(celCol.v>1){
				toPrint='+';
			}else if(celCol.v>.9){
				toPrint='#';
			}else if(celCol.v>.7){
				toPrint='x';
			}else if(celCol.v>.6){
				toPrint='.';
			}
			printf("%s%c " ,c,toPrint);

		}
		printf("\n");
	}
}
//calculate cells
struct colour* generateScreen(int w, int h,struct ball *o, int o_size){
	struct colour *screendata= malloc(w*h*sizeof(struct colour));
	for (int yy = 0; yy < h; yy++){
		for (int xx = 0; xx < w; xx++){
			struct colour v = {0,0,0};
			for(int i = 0; i< o_size;i++){
				float vv= val(xx,yy,o[i]);
				//clamp value
				if(isinf(vv) ||vv>2){
					vv=2;
				}
				//avrage hue and saturation, does not take into account hue mizing affectign saturation
				if(v.v+vv>0){
					v.h=(v.h*v.v + o[i].h*vv)/(v.v+vv);
					v.s=(v.s*v.v + o[i].s*vv)/(v.v+vv);
				}
				v.v+=vv*o[i].v;
			}
			screendata[xx+yy*w]=v;
		}
	}
	return screendata;
}
int main(){
	//get terminal size
	struct winsize ww;
	ioctl(0, TIOCGWINSZ, &ww);
    	int h = ww.ws_row/1-1;
    	int w = ww.ws_col/2;

	//seed rand
	time_t t;
	srand((unsigned) time(&t));

	//fun vars
	float simspeed=5;
	int fps = 30;
	float density=1;
	float radMod=1.5;
	//generate ammound of balls to a fitting ammount depending on window size
	int balls=w*h/400*density;
	struct ball *o = malloc(sizeof(struct ball)*balls);
	for(int i = 0; i < balls; i++){
		o[i]=generateBall(w,h,simspeed, radMod);
	}
	
	//clean screen
	printf("\e[1;1H\e[2J");
	printf("\n");
	//main loop
	while(1){
		//goto top
		printf("\033[%d;%dH", (0), (00));
		
		updateBalls(o,balls,w,h);
		//generate cells
		struct colour* s =generateScreen(w,h,o,balls);
		
		//render cells to screen
		drawScreen(s,w,h);

		free(s);
		//horrible lazy fps limiter
		usleep(1000000/fps);
		
	}
	free(o);
	return 1;
}


